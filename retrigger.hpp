// Variations: stutter (build 046), a note retrigger. While a stutter lane's
// window plays, each sequenced note on one of its tracks is struck again.
// Build 047 (2026-11-23): the lane's line is the density and its rate the
// top speed. The rate gives the levels: 1x one (a beat apart, 4 steps), 2x
// two (the beat or 2 steps), 4x three (down to a step), 8x four (down to half
// a step). The line's value says which: 128 (as set) none, 255 the top one,
// in between the level whose share of the way it has reached (rounded up). A
// lane with no points (as build 046 saved it) is at full density throughout.
// Pure and integer.
#pragma once
#include "window.hpp"

namespace variations {
const u32 kMaxRepeats = 16;

// How many levels a rate has: log2(rate) + 1.
inline u32 retrigger_levels(u32 rate) { return rate >= 8 ? 4 : rate >= 4 ? 3 : rate >= 2 ? 2 : 1; }

// A density value (Q8, 0..255 * 256, line.hpp) as a level 0..n: 0 at or below
// as set, n at the top, ceil of the share of the way between.
inline u32 retrigger_level(u32 value, u32 n) {
    if (value <= kMiddle) return 0;
    const u32 span = (255 - 128) << 8, f = value - kMiddle;
    const u32 level = (f * n + span - 1) / span;
    return level < n ? level : n;
}

// Pattern ticks between repeats at level 1..n: a beat at level 1, halving each level.
inline u32 retrigger_interval(u32 level) { return (4 * kStepTicks) >> (level ? level - 1 : 0); }

// The stutter lane (store order, lane A first) whose window covers `s` and
// whose tracks include `track`: its rate, amount and line, the window time at
// the note's start (`at`), and the ticks left to the window's end.
struct Stutter {
    u32 rate, amount, remaining, at, count, length;  // length: the window's steps (a long window stretches the line)
    const Point* points;  // the line (store bytes), `count` of them; none: full density
};
inline bool find_stutter(const Store& store, const SongPosition& s, u32 track, Stutter* out) {
    for (u32 i = 0; i < store.count(); ++i) {
        const Variation& v = store[i];
        u32 at;
        if (!window_at(v, s, &at, &store)) continue;
        for (u32 n = 0; n < kLanes; ++n) {
            const Lane& l = v.lanes[n];
            if (!is_stutter(l.gesture) || l.muted || !(l.selection >> track & 1)) continue;
            const u32 span = v.length * kStepTicks;
            u32 count;
            const Point* p = store.points(v, n, &count);
            *out = Stutter{l.rate, l.amount, at < span ? span - at : 0, at, count, v.length, p};
            return true;
        }
    }
    return false;
}

// One repeat of a note: ticks after it, its velocity and its length.
struct Repeat {
    u32 delay, velocity, length;
};
// The repeats of a note (velocity `velocity`, length `length`) in `room` ticks
// (the nearer of the next note and the window's end), into out[kMaxRepeats]; how
// many. Walking on from the note's start by t ticks: the level is read at the
// window time there (the note's own `at` + t); at level 0 the walk goes on a step
// and looks again, else the next repeat falls on the next multiple of that
// level's interval after t, if room remains, its velocity falling with t over
// the room (v * (255 - amount * t / room) / 255, at least 1) and its length the
// note's or the interval, the shorter. It stops when room runs out or
// kMaxRepeats are made.
inline u32 retrigger_plan(const Stutter& s, u32 velocity, u32 length, u32 room, Repeat* out) {
    const u32 n = retrigger_levels(s.rate);
    u32 count = 0, t = 0;
    while (count < kMaxRepeats) {
        const u32 level = s.count ? retrigger_level(value_late(s.points, s.count, s.at + t, 0, 1, s.length), n) : n;
        if (!level) {
            t += kStepTicks;
            if (t >= room) break;
            continue;
        }
        const u32 interval = retrigger_interval(level);
        t = (t / interval + 1) * interval;
        if (t >= room) break;
        const u32 v = velocity * (255 - s.amount * t / room) / 255;
        out[count++] = Repeat{t, v ? v : 1, length < interval ? length : interval};
    }
    return count;
}
}
