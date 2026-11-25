// Variations: a lane's points from its gesture (the design's gestures;
// notes/variations-format.md). Pure and integer only: the page calls it on
// every change, and tools/variations.py mirrors it (lane_lines) so the JSON
// tool writes the same points. tests/gestures.cpp holds both to the same bytes.
//
// A gesture is a shape that works on any target: a lane's value is 0..255
// with 128 "as set" (the knob where it is), 0 the knob's counter-clockwise end
// (silent for the volume, no notes, closed for the cutoff) and 255 its
// clockwise end. Amount (0..255) is how spread out entries and exits are, for
// drop the gap, for the rest how far.
//
//   build    the lane's tracks start at the bottom and come in to as set one
//            after another (lowest track first: the stagger, stagger.hpp)
//   break    the mirror: they drop to the bottom one after another (highest
//            first) and stay there; they come back suddenly where the window
//            ends (playback, not points)
//
// Format version 5 (build 024): one line a lane, all its tracks the same;
// build's and break's line is the leading track's, the others following it
// later (stagger.hpp).
//   drop     they cut to the bottom near the end; the gap is a beat (4 steps)
//            to a bar (16) by amount
//   rise     leaves as set, up by the amount, slow start (build 036: it
//            should start from the knob's value; 015-035 arrived at as set
//            from below, as fade in does). Saved points keep their shape until
//            the lane is regenerated.
//   fall     leaves as set, down by the amount, fast start: a straight line
//            sounds late (probe 088)
//   swell    up from as set and back by the window's end
//   dip      down from as set and back by the window's end
//   fade in  always on the volume (its target is fixed): from silence (100 %;
//            the amount is the depth, 50 % from halfway down) up to the
//            volume as set by the window's end, slow start (build 016)
//   fade out always on the volume: from as set down to silence (by the
//            amount) by the window's end, fast start (build 016)
//   random   (build 040) a new value at every beat (2 beats in an 8-bar
//            window, 4 in a 16-bar one), held, then a jump: as set plus
//            or minus a random amount up to the amount's reach (up_by)
//   wander   (build 040) smooth: a point every 2 beats at as set plus or
//            minus a random amount, straight lines between, back at as set
//            by the window's end
//   cut in   (build 040) the inverse of drop: at the bottom, then a jump to
//            as set near the start (drop's gap); on the volume
//   gate     (build 040; 044; named stutter until 046) a gate over the whole window, a beat on and a
//            beat off, starting on; the rate its speed, the amount how far the
//            off beats dip; one bar stored, repeated every bar; on the volume
//   stutter  (build 046) a note retrigger: each note played on its tracks while
//            the window plays is struck again, the lane's rate the top speed
//            (retrigger.hpp), the amount how far the repeats fade. Build 047:
//            its line is the density, 128 none and 255 the top speed: by
//            default a climb over the first three quarters that springs back
//            (a damped bounce) to none; the line drives nothing (it is on the
//            notes, but window.hpp skips stutter lanes there)
//   custom   its own points: never generated
//
// Random and wander's dice are deterministic: DraftLane::seed (memory only;
// the saved points are the result) feeds an xorshift32 (Rng below, mirrored
// by tools/variations.py), so the same seed gives the same values.
//
// A lane's curve (store.hpp default_curve) bends the shape, slow start to
// fast start: rise, fall and the fades bend their ramp by it; swell and dip their way
// out by it and their way back by its mirror (so out fast is back slow, a
// rounded hump, as they always were); build and break spread their entries
// and exits by it, few early then many (slow start) or many early then few
// (fast start), as the motion of their mean would bend; drop ignores it (one
// cut, already placed by the amount). Each gesture's own curve is the shape
// it made before curves could be chosen.
#pragma once
#include "store.hpp"
#include "line.hpp"

namespace variations {
enum Gesture : u32 { custom = 0, build = 1, brk = 2, drop = 3, rise = 4, fall = 5, swell = 6, dip = 7, fade_in = 8, fade_out = 9, random_ = 10, wander = 11, cut_in = 12, gate = 13, stutter = 14, stutter_up = 15, stutter_spring = 16, stutter_down = 17,
               hold = 18 };  // build 062: hold ("it might be good to have a flat line gesture", "also add in the flat line / silence curve please")

// A curve that makes a step's change land at the end of that step: f ** 4.
const int kSudden = 127;
const u32 kAsSet = 128;

namespace detail {
inline void put(DraftLane& l, u32 step, u32 value, int curve) {
    if (l.count >= kDraftPoints) return;
    Point& p = l.points[l.count++];
    p.step = static_cast<u8>(step);
    p.value = static_cast<u8>(value);
    p.curve = static_cast<signed char>(curve);
}
// From `from` to `to` over the window, bent by `curve`; nothing when they're equal.
inline void ramp(DraftLane& l, u32 length, u32 from, u32 to, int curve) {
    if (from == to) return;
    put(l, 0, from, 0);
    put(l, length - 1, to, curve);
}
// From as set to `peak` at the window's middle and back by its end: out bent
// by `curve`, back by its mirror.
inline void hump(DraftLane& l, u32 length, u32 peak, int curve) {
    if (peak == kAsSet) return;
    put(l, 0, kAsSet, 0);
    put(l, length / 2, peak, curve);
    put(l, length - 1, kAsSet, -curve);
}
// Holds `from`, then jumps to `to` at step `at` (1..length-1) and holds it.
inline void jump(DraftLane& l, u32 at, u32 from, u32 to) {
    put(l, 0, from, 0);
    if (at > 1) put(l, at - 1, from, 0);
    put(l, at, to, kSudden);
}
}

// Build 040: the dice. xorshift32 seeded with seed ^ 0x9e3779b9 (1 if that is 0).
struct Rng {
    u32 s;
    explicit Rng(u32 seed) : s(seed ^ 0x9e3779b9u) {
        if (!s) s = 1;
    }
    u32 next() {
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        return s;
    }
    // as set plus or minus up to `depth` (0..127): uniform, 0..255. Build 042:
    // each draw is a fraction (16 bits) scaled by the depth, so a new amount
    // deepens or flattens the same pattern (2026-11-23); 040-041 took a
    // remainder of the depth's range, so every amount gave other values.
    u32 around(u32 depth) {
        const u32 f = next() >> 16;
        const i32 r = static_cast<i32>((f * (2 * depth + 1)) >> 16) - static_cast<i32>(depth);
        return static_cast<u32>(static_cast<i32>(kAsSet) + r);
    }
};

// The drop's gap before the window's end, in steps. Build 049: a real-time gap (4..16
// window steps) in a window of `length` steps; on a line stretched over a window past
// 256 steps (line_length) it is given in line steps, a line step being length / 256
// window steps, and at least one (so in a 1024-step window the gap is 1..4 line steps).
inline u32 drop_gap(u32 amount, u32 length) {
    u32 gap = 4 + (amount * 12 + 127) / 255;
    if (gap > length - 1) gap = length - 1;
    if (length > 256) {
        gap = gap * 256 / length;
        if (gap < 1) gap = 1;
    }
    return gap;
}
// How far up (above as set) or down an amount reaches.
inline u32 up_by(u32 amount) { return kAsSet + (amount * 127 + 127) / 255; }
inline u32 down_by(u32 amount) { return kAsSet - (amount * 128 + 127) / 255; }

// A lane's line from its gesture, amount and tracks, in a window of `length`
// steps (custom keeps its own; an empty lane has none). Format version 5: one
// line a lane, which all its tracks follow; build's and break's tracks follow
// it one after another (stagger.hpp), so their line is the leading track's
// entry or exit. The lane's curve bends the shape (store.hpp default_curve;
// drop has none).
//
// Build 049 (version 9): the points are in line steps, over line_length(window_length):
// the window's own up to 256 steps, 256 stretched over a longer window. So everything
// below draws over `length`, the line's, but for drop's and cut in's gap, which is real
// time (drop_gap).
inline void generate(DraftLane& l, u32 window_length) {
    if (l.gesture == custom) return;
    const u32 length = line_length(window_length);
    clear_points(l);
    if (!l.selection) return;  // no tracks: nothing to move, no line (as build 023)
    const u32 a = l.amount;
    switch (l.gesture) {
    case build:
    case brk: l.count = static_cast<unsigned char>(entry_line(l.gesture, a, l.curve, l.selection, window_length, l.points)); break;
    case drop: detail::jump(l, length - drop_gap(a, window_length), kAsSet, 0); break;
    case rise:
    case fall:
    case fade_in:
    case fade_out: {
        // Build 036 (2026-11-23): rise leaves as set, up by the amount
        // (015-035 arrived at as set from below, a fade in); fall leaves it,
        // down. Fade in arrives at as set from below; fade out leaves it, down.
        if (l.gesture == rise) {
            detail::ramp(l, length, kAsSet, up_by(a), l.curve);
            break;
        }
        const bool in = l.gesture == fade_in;
        detail::ramp(l, length, in ? down_by(a) : kAsSet, in ? kAsSet : down_by(a), l.curve);
        break;
    }
    case random_:
    case wander: {
        if (a == 0) break;
        Rng rng(l.seed);
        const u32 depth = up_by(a) - kAsSet;
        if (l.gesture == random_) {
            // Sample and hold: each value held to the step before the next, then a
            // jump (a hold point and the new value, as drop's cut). A beat apart,
            // two in an 8-bar window and four in a 16-bar one (build 047), so the
            // points stay within 32.
            const u32 every = length > 128 ? 16 : length > 64 ? 8 : 4;
            u32 held = rng.around(depth);
            detail::put(l, 0, held, 0);
            for (u32 s = every; s < length; s += every) {
                detail::put(l, s - 1, held, 0);
                held = rng.around(depth);
                detail::put(l, s, held, kSudden);
            }
            break;
        }
        detail::put(l, 0, kAsSet, 0);
        // Every 2 beats, 4 in a window past 8 bars (build 047: 16 bars, 32 points).
        const u32 every = length > 128 ? 16 : 8;
        for (u32 s = every; s < length - 1; s += every) detail::put(l, s, rng.around(depth), 0);
        detail::put(l, length - 1, kAsSet, 0);
        break;
    }
    case cut_in: detail::jump(l, drop_gap(a, window_length), 0, kAsSet); break;
    case gate: {
        // Build 044 (2026-11-23): a gate over the whole window, a beat on
        // and a beat off, starting on; the lane's rate sets its speed (2x two
        // steps, 4x a step), the amount how far the off beats dip (full:
        // silent). One bar is stored, stretched over the window's steps
        // (stutter_scale), and playback repeats it every bar (window.hpp), so
        // 8 bars fit in 32 points. Build 049: in a window past 256 steps the bar is
        // stored over the 256-step line (scale 16) and played at rate * stutter_scale(
        // window length) repeats, so each repeat is still a real bar, a beat on and a
        // beat off at 1x.
        if (!a) break;
        const u32 cycle = length < 16 ? length : 16, scale = stutter_scale(length);
        // Each beat held to the step before the next, then a jump (as drop's cut),
        // so it gates rather than glides.
        for (u32 k = 0; k * 4 < cycle; ++k) {
            const u32 at = k * 4 * scale;
            if (k) detail::put(l, at - 1, k % 2 ? kAsSet : down_by(a), 0);
            detail::put(l, at, k % 2 ? down_by(a) : kAsSet, k ? kSudden : 0);
        }
        break;
    }
    // Build 048 (2026-11-23): 047 made the spring stutter's only shape; that
    // wanted 046's steady stutter as it was, with the shapes as variations. The
    // line is the density (128: no repeat, 255: the top speed), whatever the amount.
    case stutter:  // steady: the top all the way
        detail::put(l, 0, 255, 0);
        detail::put(l, length - 1, 255, 0);
        break;
    case stutter_up:  // more and more stuttery, a slow start
        detail::put(l, 0, kAsSet, 0);
        detail::put(l, length - 1, 255, kSlowStart);
        break;
    case stutter_down:  // stuttering hard, easing off to plain notes
        detail::put(l, 0, 255, 0);
        detail::put(l, length - 1, kAsSet, kFastStart);
        break;
    case stutter_spring: {
        // Build 047's line: a slow-start
        // climb to the top at three quarters of the window, then a damped
        // spring back to none: 170, 225, 150, 128 over the last quarter. A
        // point whose step is not past the one before is left out (short
        // windows); the first point and the peak stay.
        const u32 p = length * 3 / 4, q = length - 1 - p;
        detail::put(l, 0, kAsSet, 0);
        detail::put(l, p, 255, kSlowStart);
        const u32 steps[4] = {p + q / 4, p + q / 2, p + 3 * q / 4, length - 1}, values[4] = {170, 225, 150, kAsSet};
        u32 last = p;
        for (u32 k = 0; k < 4; ++k) {
            if (steps[k] <= last) continue;
            detail::put(l, steps[k], values[k], 0);
            last = steps[k];
        }
        break;
    }
    case hold:
        // Build 062 ("it might be good to have a flat line gesture", "also add in the flat line / silence
        // curve please"): a flat line from the window's first step to its last, at the amount's depth below
        // as set (full amount: 0, silence on a volume). Like dip's hump it is left out when the amount leaves
        // the line at as set; on a one-step window the two ends are one step, so it is one point.
        if (down_by(a) == kAsSet) break;
        detail::put(l, 0, down_by(a), 0);
        if (length > 1) detail::put(l, length - 1, down_by(a), 0);
        break;
    case swell:
    case dip: detail::hump(l, length, l.gesture == swell ? up_by(a) : down_by(a), l.curve); break;
    default: break;  // an empty lane
    }
}
inline void generate(Draft& d) {
    for (DraftLane& l : d.lanes) generate(l, d.length);
}

// Default tracks (the design): over a join (the window starts before it),
// build takes the tracks arriving in the section after and break the tracks
// leaving the one before; inside a section, and when none arrive or leave,
// build and break take the percussion group playing there. Every other
// gesture takes every playing track. `before`, `after`: the tracks heard in
// each section (0: unknown); `percussion`: Drum and Drumline tracks known.
// With nothing known, every track.
inline u32 default_tracks(u32 gesture, bool over_join, u32 before, u32 after, u32 percussion) {
    const u32 all = 0xff;
    const u32 playing_after = after ? after : all;
    const u32 playing = over_join ? (before | after ? before | after : all) : playing_after;
    if (gesture != build && gesture != brk) return playing;
    if (over_join && before && after) {
        const u32 moving = gesture == build ? after & ~before : before & ~after;
        if (moving) return moving;
    }
    const u32 section = over_join && gesture == brk ? (before ? before : all) : playing_after;
    const u32 drums = section & percussion;
    return drums ? drums : section;
}
}
