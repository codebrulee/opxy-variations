// Variations: build's and break's one-after-another (format version 5).
// A lane has one line, and all its tracks follow it; build and break stagger
// them: each track follows the lane's line shifted later than the one before
// (build the lowest track first, break the highest), as far apart as the
// amount spreads them (all together at none, across the window at full) and
// bent by the lane's curve. The line is the leading track's (no shift), so the
// stagger puts the r-th of n tracks' entry or exit at turn_step(r, n, ...),
// where version 4 stored a jump per track. Pure and integer only: playback
// (window.hpp), the page (gestures.hpp) and the store's upgrade use it, and
// tools/variations.py mirrors it.
#pragma once
#include "line.hpp"

namespace variations {
const u32 kGestureBuild = 1, kGestureBreak = 2;

inline u32 count_bits(u32 mask) {
    u32 n = 0;
    for (; mask; mask &= mask - 1) ++n;
    return n;
}

// Where the r-th of n tracks enters (build) or leaves (break), in steps into a
// window of `length`: evenly spread across it at full amount, all together in
// the middle at none, snapped to the beat in windows of a bar or more. A
// curve bends the spread: above 0 (slow start) the turns come later and closer
// together towards the end, below 0 earlier, so the lane's motion bends as a
// ramp's would (the r-th's place p = (2r + 1) / 2n becomes p ** 2^(-2c), the
// inverse of a ramp's bend).
inline u32 turn_step(u32 r, u32 n, u32 amount, u32 length, int curve = 0) {
    u32 step;
    if (!curve) {
        // length * (1/2 + ((2r + 1) / 2n - 1/2) * amount / 255), rounded.
        const u32 num = length * (n * 255 + (2 * r + 1) * amount) - length * n * amount;
        const u32 den = 2 * n * 255;
        step = (num + den / 2) / den;
    } else {
        // The same with p bent, Q16: length * (1/2 * (255 - amount) + p * amount) / 255.
        const u32 p = bend(((2 * r + 1) << 16) / (2 * n), -curve);
        const u32 num = length * (32768u * (255 - amount) + p * amount), den = 65536u * 255;
        step = (num + den / 2) / den;
    }
    const u32 grid = length >= 16 ? 4 : 1;
    step = (step + grid / 2) / grid * grid;
    if (step < grid) step = grid;
    if (step > length - grid) step = length - grid;
    return step;
}

inline bool staggers(u32 gesture) { return gesture == kGestureBuild || gesture == kGestureBreak; }

// A track's place in the stagger: 0 the leading track (build: the lane's
// lowest; break: its highest; any other gesture: its lowest).
inline u32 stagger_rank(u32 gesture, u32 selection, u32 track) {
    const u32 below = selection & ((1u << track) - 1);
    return gesture == kGestureBreak ? count_bits(selection >> track >> 1) : count_bits(below);
}
// The leading track (-1: no tracks).
inline i32 leading_track(u32 gesture, u32 selection) {
    for (u32 t = 0; t < 8; ++t) {
        const u32 track = gesture == kGestureBreak ? 7 - t : t;
        if (selection >> track & 1) return static_cast<i32>(track);
    }
    return -1;
}

// How many line steps later than the lane's line `track` follows it: 0 but for
// build and break (edited ones too: the gesture is kept), and a track of the
// lane's. Build 049: `length` is the window's; the spread is worked out over the
// line's own length (line_length), since the shift is applied in line steps.
inline u32 stagger(u32 gesture, u32 amount, int curve, u32 selection, u32 track, u32 window_length) {
    if (!staggers(gesture) || track >= 8 || !(selection >> track & 1)) return 0;
    const u32 length = line_length(window_length);
    const u32 n = count_bits(selection), r = stagger_rank(gesture, selection, track);
    if (!r) return 0;
    return turn_step(r, n, amount, length, curve) - turn_step(0, n, amount, length, curve);
}

// The line's value at `at` (scene clock units into the window) for a track
// following it `shift` line steps late: before the shift, the line's first value.
//
// Version 7 (build 041): at `rate` 2, 4 or 8 the line plays that many times
// across the window of `length` steps: `at` is first mapped to the line's own
// time, (at * rate) mod span, and the shift applies in that time.
//
// Build 049 (version 9): in a window past 256 steps the line (line_length(length)
// steps) is stretched over it: `at` is read at at * line_length / length. In u32:
// at < 1024 * 480 and the line is at most 256 steps, so the product stays under 2^27.
// Build 063 (version 12): windows to 4096 steps, so at < 4096 * 480 and the product stays under 2^29; a gate's
// rate is its lane's times stutter_scale(length), at most 8 * 256, so the rate step below reduces `at` to the span
// first (the same result mod the span) and (at * rate) stays under 4096 * 480 * 2048, 4.03e9, inside a u32.
// The shift is in line steps (stagger()'s `length` is the line's). `length` 0 (the
// callers that read a line as it is) leaves `at` alone.
inline u32 value_late(const Point* points, u32 count, u32 at, u32 shift, u32 rate = 1, u32 length = 0) {
    const u32 late = shift * kStepTicks;
    if (rate > 1) {
        const u32 span = length * kStepTicks;
        at = at % span * rate % span;
    }
    if (length > 256) at = at * 256 / length;  // at < span here, or a little past it in an ease's tail
    return value_at(points, count, at > late ? at - late : 0);
}

// Build's or break's line (the leading track's): holds `from`, then jumps to
// `to` at its turn and holds it, the jump landing at the end of its step
// (curve 127). Into `out` (at most 3 points); returns how many. n tracks (none
// counts as one, so a lane without tracks still has its line). Build 049: over
// the line's length (line_length) of the window `window_length`.
const int kJumpCurve = 127;
inline u32 entry_line(u32 gesture, u32 amount, int curve, u32 selection, u32 window_length, Point* out) {
    const u32 length = line_length(window_length);
    const u32 n = count_bits(selection), at = turn_step(0, n ? n : 1, amount, length, curve);
    const u8 from = gesture == kGestureBuild ? 0 : 128, to = gesture == kGestureBuild ? 128 : 0;
    u32 k = 0;
    out[k++] = Point{0, from, 0};
    if (at > 1) out[k++] = Point{static_cast<u8>(at - 1), from, 0};
    out[k++] = Point{static_cast<u8>(at), to, static_cast<signed char>(kJumpCurve)};
    return k;
}
}
