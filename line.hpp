// Variations: a line's value at a position in its window (notes/variations-format.md).
// Integer only (the main board has no FPU): this runs for every line of every
// selected track on every controller pass, 689 times a second.
//
// A line holds its first point's value before it and its last point's after.
// Between two points it runs from one value to the next, bent by the curve of
// the point it runs into: f ** 2^(2c), c = curve / 127, f the fraction of the
// way along (0 straight; above 0 a slow start, below 0 a fast start), as the
// variation page's mockup (mockups/transition.html, valueAt).
#pragma once
#include <opxy/types.hpp>

namespace variations {
using opxy::i32;
using opxy::u32;
using opxy::u8;
using opxy::u16;

struct Point {
    u8 step;   // within the window, 0..length-1
    u8 value;  // 0..255
    signed char curve;  // -127..127
};

const u32 kStepTicks = 480;  // scene clock units a step (1/20 transport tick)
// Build 049 (format version 9): a window may be up to 1024 steps (4096 from build 063, version 12), but a point's
// step is a u8, so a line has its own length: the window's, up to 256 steps, and
// 256 steps stretched over any longer window (stagger.hpp value_late). For a
// window of 256 steps or fewer, line steps are window steps.
inline u32 line_length(u32 length) { return length <= 256 ? length : 256; }
const u32 kOne = 65536;      // Q16 one
const u32 kMiddle = 128 << 8;  // a line's middle, Q8: notes "as written", filter "open"

namespace detail {
// log2(1 + i/32) and 2^(i/32), Q16, i = 0..32.
const u32 kLog2[33] = {0, 2909, 5732, 8473, 11136, 13727, 16248, 18704, 21098, 23433, 25711,
                       27936, 30109, 32234, 34312, 36346, 38336, 40286, 42196, 44068, 45904,
                       47705, 49472, 51207, 52911, 54584, 56229, 57845, 59434, 60997, 62534,
                       64047, 65536};
const u32 kExp2[33] = {65536, 66971, 68438, 69936, 71468, 73032, 74632, 76266, 77936, 79642,
                       81386, 83169, 84990, 86851, 88752, 90696, 92682, 94711, 96785, 98905,
                       101070, 103283, 105545, 107856, 110218, 112631, 115098, 117618, 120194,
                       122825, 125515, 128263, 131072};

// -log2(f) in Q16 for f in (0, 1] (Q16, 1..65536).
inline u32 neg_log2(u32 f) {
    u32 whole = 0;
    while (f < kOne) {
        f <<= 1;
        ++whole;
    }
    // f in [1, 2) as Q16: interpolate log2 of its fraction.
    const u32 frac = f - kOne, i = frac >> 11, rest = frac & 2047;
    const u32 l = kLog2[i] + ((kLog2[i + 1] - kLog2[i]) * rest >> 11);
    return (whole << 16) - l;
}

// 2^(-x) in Q16 for x >= 0 (Q16).
inline u32 exp2_neg(u32 x) {
    const u32 whole = x >> 16;
    if (whole >= 16) return 0;
    const u32 frac = (kOne - (x & 0xffff)) & 0xffff;  // 2^-x = 2^(frac) / 2^(whole + (frac != 0))
    const u32 shift = whole + (frac ? 1 : 0);
    const u32 i = frac >> 11, rest = frac & 2047;
    const u32 e = kExp2[i] + ((kExp2[i + 1] - kExp2[i]) * rest >> 11);
    return e >> shift;
}

// 2^(2c) in Q16, c = curve / 127: from 1/4 to 4.
inline u32 exponent(int curve) {
    const i32 x = 2 * curve * 65536 / 127;  // Q16 log2 of the exponent, -2..2
    if (x >= 0) return (exp2_neg((2 << 16) - static_cast<u32>(x)) << 2);  // 2^x = 4 * 2^-(2-x)
    return exp2_neg(static_cast<u32>(-x));
}
}

// f ** 2^(2c), f and the result Q16 (0..65536).
inline u32 bend(u32 f, int curve) {
    if (curve == 0 || f == 0 || f >= kOne) return f;
    const u32 e = detail::exponent(curve);
    // -log2(f) * e, both Q16; split to stay in 32 bits.
    const u32 l = detail::neg_log2(f);  // up to 16 << 16 for f >= 1/65536
    // x = l * e (both Q16) in 32 bits: l < 2^21 and e <= 2^18, so split e at bit 10.
    const u32 x = ((l * (e >> 10)) >> 6) + ((l * (e & 1023)) >> 16);
    return detail::exp2_neg(x);
}

// The line's value at `at` (scene clock units from the window's start), Q8:
// 0..255 * 256. points: 1 or more, in step order.
inline u32 value_at(const Point* points, u32 count, u32 at) {
    if (!count) return 0;
    if (at <= points[0].step * kStepTicks) return points[0].value << 8;
    for (u32 i = 1; i < count; ++i) {
        const u32 to = points[i].step * kStepTicks;
        if (at > to) continue;
        const u32 from = points[i - 1].step * kStepTicks;
        // f = (at - from) / (to - from), Q16, in 32 bits: spans are whole steps (480 = 30 * 16)
        // of at most 256 steps (122880: build 047), so (at - from) << 12 fits and (to - from) >> 4 is exact.
        const u32 f = ((at - from) << 12) / ((to - from) >> 4);
        const u32 k = bend(f, points[i].curve);
        const i32 a = points[i - 1].value << 8, z = points[i].value << 8;
        return static_cast<u32>(a + ((z - a) * static_cast<i32>(k >> 1) >> 15));
    }
    return points[count - 1].value << 8;
}
}
