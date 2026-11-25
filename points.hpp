// Variations: editing a lane's line (the shape page,
// mockups/variation-points.html). Pure and integer only, on a DraftLane: the
// page calls these and then puts the draft in the store.
//
// A lane's line is its points in step order (store.hpp DraftLane; format
// version 5: one a lane, all its tracks following it), each a value 0..255
// (128 as set) and the curve of the segment into it (line.hpp). A line with
// no points holds as set.
#pragma once
#include "store.hpp"
#include "line.hpp"

namespace variations {
namespace edit {
// The point at `step`, or -1.
inline i32 find(const DraftLane& l, u32 step) {
    for (u32 k = 0; k < l.count; ++k)
        if (l.points[k].step == step) return static_cast<i32>(k);
    return -1;
}
// The line's value at `step`, 0..255 (as set where the line has no points).
inline u32 value(const DraftLane& l, u32 step) {
    if (!l.count) return 128;
    return (value_at(l.points, l.count, step * kStepTicks) + 128) >> 8;
}
// A point at `step` where the line is, the curve into it the next point's (so
// the segment it splits keeps its bend); its index, or -1 when the line holds
// its most (kDraftPoints) or there is one there already.
inline i32 add(DraftLane& l, u32 step) {
    if (find(l, step) >= 0 || l.count >= kDraftPoints) return -1;
    const u32 v = value(l, step);
    Point* p = l.points;
    u32 at = 0;
    while (at < l.count && p[at].step < step) ++at;
    const signed char curve = at < l.count && at > 0 ? p[at].curve : 0;
    for (u32 k = l.count; k > at; --k) p[k] = p[k - 1];
    p[at].step = static_cast<u8>(step);
    p[at].value = static_cast<u8>(v);
    p[at].curve = curve;
    ++l.count;
    return static_cast<i32>(at);
}
// The point at `step` removed; false when there is none.
inline bool remove(DraftLane& l, u32 step) {
    const i32 k = find(l, step);
    if (k < 0) return false;
    Point* p = l.points;
    for (u32 i = static_cast<u32>(k); i + 1 < l.count; ++i) p[i] = p[i + 1];
    --l.count;
    return true;
}
// The next point after `step` (dir > 0) or the one before it; -1 for none.
inline i32 next(const DraftLane& l, u32 step, i32 dir) {
    if (dir > 0) {
        for (u32 k = 0; k < l.count; ++k)
            if (l.points[k].step > step) return l.points[k].step;
    } else {
        for (u32 k = l.count; k-- > 0;)
            if (l.points[k].step < step) return l.points[k].step;
    }
    return -1;
}

// A value as the page shows it: as set (0), or the way to the knob's
// counter-clockwise end (-100 %, value 0) or its clockwise end (+100 %, 255).
inline i32 percent(u32 v) {
    if (v < 128) return -static_cast<i32>(((128 - v) * 100 + 64) / 128);
    return static_cast<i32>(((v - 128) * 100 + 63) / 127);
}
inline u32 from_percent(i32 p) {
    if (p < 0) return 128 - static_cast<u32>((-p * 128 + 50) / 100);
    return 128 + static_cast<u32>((p * 127 + 50) / 100);
}
// A value `by` percent on, within -100..100 %.
inline u32 nudge(u32 v, i32 by) {
    i32 to = percent(v) + by;
    if (to < -100) to = -100;
    if (to > 100) to = 100;
    return from_percent(to);
}
// A curve `detents` steps of 5 % of the most on (to the 5 % grid first): right
// a faster start (below 0), left a slower one, as the variation page's curve.
inline i32 curve_step(i32 curve, i32 detents) {
    const i32 c = -curve, mag = (c < 0 ? -c : c) * 100, now = (mag + 63) / 127 * (c < 0 ? -1 : 1);
    const i32 base = (now >= 0 ? now / 5 : -((-now + 4) / 5)) * 5;
    i32 to = detents > 0 ? base + detents * 5 : (now == base ? base : base + 5) + detents * 5;
    if (to < -100) to = -100;
    if (to > 100) to = 100;
    return -(to < 0 ? -((-to * 127 + 50) / 100) : (to * 127 + 50) / 100);
}
}
}
