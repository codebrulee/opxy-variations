// Host test: gestures.hpp's points, the same bytes as tools/variations.py's
// lane_line (tests/fixtures/gestures.json; test_variations_tool.py keeps the
// bytes below current), and what each gesture does on any target. Format
// version 5: one line a lane; each track's line here is the lane's as that
// track follows it (build's and break's shifted by the stagger, stagger.hpp),
// so the checks read as build 023's per-track lines did.
#include "../gestures.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
using namespace variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
struct LaneCase { u32 gesture, amount, target, selection; bool tracks, follows, bent; int curve; };  // bent: its own curve, else the gesture's
struct Case { u32 song, entry; i32 offset; u32 length; LaneCase lanes[2]; };
const LaneCase none = {kNoGesture, 128, kNoTarget, 0, false, true};
// fixtures/gestures.json, field for field.
static const Case cases[] = {
    {0, 1, -32, 32, {{build, 128, kVolume, 0x07, true, true}, {rise, 255, kCutoff, 0x18, false, false}}},
    {1, 2, -32, 64, {{brk, 255, kVolume, 0x0f, false, true}, {fall, 128, 24, 0x20, false, false}}},
    {2, 3, 0, 8, {{brk, 0, kVolume, 0x80, false, true}, none}},
    {0, 4, -32, 32, {{drop, 128, kVolume, 0xff, true, true}, {swell, 200, kSend, 0x0c, false, false}}},
    {1, 5, -8, 8, {{drop, 255, kTargetNotes, 0x02, false, false}, {dip, 255, 11, 0x40, false, false}}},
    {2, 6, 16, 16, {{swell, 0, kCutoff, 0x04, false, true}, none}},
    {0, 7, -128, 128, {{rise, 255, kCutoff, 0x10, false, true}, {build, 0, 1, 0x03, false, false}}},
    {1, 8, -16, 32, {{fall, 128, kCutoff, 0x30, false, true}, {dip, 64, 40, 0x80, false, false}}},
    // Curves of their own (version 4).
    {0, 9, -64, 64, {{build, 255, kVolume, 0x0f, false, true, true, 90}, {brk, 200, kVolume, 0xf0, false, true, true, -70}}},
    {1, 10, -32, 32, {{rise, 255, kVolume, 0x01, false, false, true, -30}, {swell, 255, kCutoff, 0x02, false, false, true, 64}}},
    {2, 11, -32, 32, {{fall, 128, kCutoff, 0x04, false, true, true, 0}, {dip, 128, kCutoff, 0x08, false, false, true, 127}}},
    // The fades (build 016): always the volume, their own curves or one given.
    {0, 12, 0, 32, {{fade_in, 255, kVolume, 0x03, true, true}, {fade_out, 128, kVolume, 0x04, false, true}}},
    {1, 13, -16, 16, {{fade_out, 255, kVolume, 0x08, false, true, true, 30}, {fade_in, 128, kVolume, 0x10, false, false, true, -50}}},
    // Build 040: random, wander, cut in, gate (seed 0).
    {0, 14, -32, 32, {{random_, 200, kCutoff, 0x03, false, true}, {wander, 255, kCutoff, 0x04, false, true}}},
    {1, 15, -128, 128, {{random_, 255, kCutoff, 0x01, false, true}, {wander, 0, kCutoff, 0x02, false, true}}},
    {2, 16, -16, 16, {{cut_in, 128, kVolume, 0x07, true, true}, {gate, 255, kVolume, 0x08, false, true}}},
    {0, 17, -32, 32, {{gate, 0, kVolume, 0x01, false, true}, {cut_in, 255, kVolume, 0x02, false, true}}},
    // Build 046: stutter (gesture 14, no line, on the notes).
    {1, 18, -16, 32, {{stutter, 128, kTargetNotes, 0x03, false, true}, {gate, 200, kVolume, 0x04, false, true}}},
    // Build 047: windows to 256 steps (version 8).
    {2, 19, 0, 256, {{random_, 200, kCutoff, 0x03, false, true}, {wander, 255, kCutoff, 0x04, false, true}}},
    {0, 20, 64, 192, {{gate, 255, kVolume, 0x01, false, true}, {stutter, 100, kTargetNotes, 0x02, false, true}}},
    // Build 062 ("add in the flat line / silence curve please"): hold, on the shortest window.
    {1, 21, 0, 8, {{hold, 255, kVolume, 0x01, false, true}, {hold, 128, kCutoff, 0x02, false, false}}},
};
// fixtures/gestures.json as tools/variations.py encodes it.
static const u8 expected[] = {86, 82, 12, 21, 0, 1, 224, 255, 32, 0, 0, 255, 255, 1, 128, 15, 7, 3, 0, 3, 0, 0, 0, 11, 0, 0, 12, 128, 127, 4, 255, 16, 24, 0, 60, 2, 0, 128, 0, 31, 255, 60, 255, 128, 255, 0, 2, 0, 0, 1, 2, 224, 255, 64, 0, 0, 255, 255, 2, 255, 15, 15, 2, 0, 3, 0, 128, 0, 7, 128, 0, 8, 0, 127, 5, 128, 24, 32, 0, 196, 2, 0, 128, 0, 63, 64, 196, 255, 128, 255, 0, 2, 0, 0, 2, 3, 0, 0, 8, 0, 0, 255, 255, 2, 0, 15, 128, 2, 0, 3, 0, 128, 0, 3, 128, 0, 4, 0, 127, 255, 128, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0, 0, 4, 224, 255, 32, 0, 0, 255, 255, 3, 128, 15, 255, 3, 0, 3, 0, 128, 0, 21, 128, 0, 22, 0, 127, 6, 200, 22, 12, 0, 216, 3, 0, 128, 0, 16, 228, 216, 31, 128, 40, 255, 128, 255, 0, 2, 0, 0, 1, 5, 248, 255, 8, 0, 0, 255, 255, 3, 255, 64, 2, 0, 0, 2, 0, 128, 0, 1, 0, 127, 7, 255, 11, 64, 0, 216, 3, 0, 128, 0, 4, 0, 216, 7, 128, 40, 255, 128, 255, 0, 2, 0, 0, 2, 6, 16, 0, 16, 0, 0, 255, 255, 6, 0, 16, 4, 2, 216, 0, 255, 128, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0, 0, 7, 128, 255, 128, 0, 0, 255, 255, 4, 255, 16, 16, 2, 60, 2, 0, 128, 0, 127, 255, 60, 1, 0, 1, 3, 0, 0, 3, 0, 0, 0, 63, 0, 0, 64, 128, 127, 255, 128, 255, 0, 2, 0, 0, 1, 8, 240, 255, 32, 0, 0, 255, 255, 5, 128, 16, 48, 2, 196, 2, 0, 128, 0, 31, 64, 196, 7, 64, 40, 128, 0, 216, 3, 0, 128, 0, 16, 96, 216, 31, 128, 40, 255, 128, 255, 0, 2, 0, 0, 0, 9, 192, 255, 64, 0, 0, 255, 255, 1, 255, 15, 15, 2, 90, 3, 0, 0, 0, 27, 0, 0, 28, 128, 127, 2, 200, 15, 240, 2, 186, 3, 0, 128, 0, 7, 128, 0, 8, 0, 127, 255, 128, 255, 0, 2, 0, 0, 1, 10, 224, 255, 32, 0, 0, 255, 255, 4, 255, 15, 1, 0, 226, 2, 0, 128, 0, 31, 255, 226, 6, 255, 16, 2, 0, 64, 3, 0, 128, 0, 16, 255, 64, 31, 128, 192, 255, 128, 255, 0, 2, 0, 0, 2, 11, 224, 255, 32, 0, 0, 255, 255, 5, 128, 16, 4, 2, 0, 2, 0, 128, 0, 31, 64, 0, 7, 128, 16, 8, 0, 127, 3, 0, 128, 0, 16, 64, 127, 31, 128, 129, 255, 128, 255, 0, 2, 0, 0, 0, 12, 0, 0, 32, 0, 0, 255, 255, 8, 255, 15, 3, 3, 60, 2, 0, 0, 0, 31, 128, 60, 9, 128, 15, 4, 2, 196, 2, 0, 128, 0, 31, 64, 196, 255, 128, 255, 0, 2, 0, 0, 1, 13, 240, 255, 16, 0, 0, 255, 255, 9, 255, 15, 8, 2, 30, 2, 0, 128, 0, 15, 0, 30, 8, 128, 15, 16, 0, 206, 2, 0, 64, 0, 15, 128, 206, 255, 128, 255, 0, 2, 0, 0, 0, 14, 224, 255, 32, 0, 0, 255, 255, 10, 200, 16, 3, 2, 0, 15, 0, 91, 0, 3, 91, 0, 4, 204, 127, 7, 204, 0, 8, 125, 127, 11, 125, 0, 12, 29, 127, 15, 29, 0, 16, 208, 127, 19, 208, 0, 20, 223, 127, 23, 223, 0, 24, 174, 127, 27, 174, 0, 28, 226, 127, 11, 255, 16, 4, 2, 0, 5, 0, 128, 0, 8, 81, 0, 16, 224, 0, 24, 124, 0, 31, 128, 0, 255, 128, 255, 0, 2, 0, 0, 1, 15, 128, 255, 128, 0, 0, 255, 255, 10, 255, 16, 1, 2, 0, 31, 0, 81, 0, 7, 81, 0, 8, 224, 127, 15, 224, 0, 16, 124, 127, 23, 124, 0, 24, 2, 127, 31, 2, 0, 32, 230, 127, 39, 230, 0, 40, 249, 127, 47, 249, 0, 48, 186, 127, 55, 186, 0, 56, 252, 127, 63, 252, 0, 64, 169, 127, 71, 169, 0, 72, 181, 127, 79, 181, 0, 80, 27, 127, 87, 27, 0, 88, 249, 127, 95, 249, 0, 96, 110, 127, 103, 110, 0, 104, 71, 127, 111, 71, 0, 112, 13, 127, 119, 13, 0, 120, 68, 127, 11, 0, 16, 2, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0, 2, 16, 240, 255, 16, 0, 0, 255, 255, 12, 128, 15, 7, 3, 0, 3, 0, 0, 0, 9, 0, 0, 10, 128, 127, 13, 255, 15, 8, 2, 0, 7, 0, 128, 0, 3, 128, 0, 4, 0, 127, 7, 0, 0, 8, 128, 127, 11, 128, 0, 12, 0, 127, 255, 128, 255, 0, 2, 0, 0, 0, 17, 224, 255, 32, 0, 0, 255, 255, 13, 0, 15, 1, 2, 0, 0, 12, 255, 15, 2, 2, 0, 3, 0, 0, 0, 15, 0, 0, 16, 128, 127, 255, 128, 255, 0, 2, 0, 0, 1, 18, 240, 255, 32, 0, 0, 255, 255, 14, 128, 64, 3, 2, 0, 2, 0, 255, 0, 31, 255, 0, 13, 200, 15, 4, 2, 0, 7, 0, 128, 0, 7, 128, 0, 8, 28, 127, 15, 28, 0, 16, 128, 127, 23, 128, 0, 24, 28, 127, 255, 128, 255, 0, 2, 0, 0, 2, 19, 0, 0, 0, 1, 0, 255, 255, 10, 200, 16, 3, 2, 0, 31, 0, 91, 0, 15, 91, 0, 16, 204, 127, 31, 204, 0, 32, 125, 127, 47, 125, 0, 48, 29, 127, 63, 29, 0, 64, 208, 127, 79, 208, 0, 80, 223, 127, 95, 223, 0, 96, 174, 127, 111, 174, 0, 112, 226, 127, 127, 226, 0, 128, 160, 127, 143, 160, 0, 144, 170, 127, 159, 170, 0, 160, 48, 127, 175, 48, 0, 176, 223, 127, 191, 223, 0, 192, 114, 127, 207, 114, 0, 208, 83, 127, 223, 83, 0, 224, 37, 127, 239, 37, 0, 240, 81, 127, 11, 255, 16, 4, 2, 0, 17, 0, 128, 0, 16, 81, 0, 32, 224, 0, 48, 124, 0, 64, 2, 0, 80, 230, 0, 96, 249, 0, 112, 186, 0, 128, 252, 0, 144, 169, 0, 160, 181, 0, 176, 27, 0, 192, 249, 0, 208, 110, 0, 224, 71, 0, 240, 13, 0, 255, 128, 0, 255, 128, 255, 0, 2, 0, 0, 0, 20, 64, 0, 192, 0, 0, 255, 255, 13, 255, 15, 1, 2, 0, 7, 0, 128, 0, 47, 128, 0, 48, 0, 127, 95, 0, 0, 96, 128, 127, 143, 128, 0, 144, 0, 127, 14, 100, 64, 2, 2, 0, 2, 0, 255, 0, 191, 255, 0, 255, 128, 255, 0, 2, 0, 0, 1, 21, 0, 0, 8, 0, 0, 255, 255, 18, 255, 15, 1, 2, 0, 2, 0, 0, 0, 7, 0, 0, 18, 128, 16, 2, 0, 0, 2, 0, 64, 0, 7, 64, 0, 255, 128, 255, 0, 2, 0, 0};
static Store store;
static Draft d;
static void draft_of(const Case& c) {
    d.song = c.song, d.entry = c.entry, d.offset = c.offset, d.length = c.length;
    d.unplaced = false, d.scene_before = d.scene_after = kNoScene;
    clear_lane(d.lanes[2]);  // build 060: lane C stays empty (the cases are two lanes)
    for (u32 n = 0; n < kLanesV10; ++n) {
        const LaneCase& l = c.lanes[n];
        DraftLane& o = d.lanes[n];
        clear_lane(o);
        o.gesture = l.gesture, o.amount = l.amount, o.target = l.target, o.selection = l.selection;
        o.default_tracks = l.tracks, o.default_target = l.follows;
        o.curve = l.bent ? l.curve : default_curve(l.gesture);
    }
    generate(d);
}
// Track t's line in `of`: the lane's, `stagger` steps later (its first point,
// at step 0, holds from the start); null for a track not in the lane or no line.
static const Point* track_line(const Draft& of, u32 lane, u32 t, u32* n, Point* out) {
    const DraftLane& l = of.lanes[lane];
    *n = 0;
    if (!(l.selection >> t & 1) || !l.count) return nullptr;
    const u32 shift = stagger(l.gesture, l.amount, l.curve, l.selection, t, of.length);
    for (u32 k = 0; k < l.count; ++k) {
        Point p = l.points[k];
        if (k || p.step) p.step = static_cast<u8>(p.step + shift);
        if (p.step >= of.length) break;
        out[(*n)++] = p;
    }
    return out;
}
static Point buffer[2][8][kDraftPoints];
static const Point* line(u32 lane, u32 t, u32* n) { return track_line(d, lane, t, n, buffer[lane][t]); }
static u32 last_step(const Draft& of, u32 lane, u32 t) {
    Point out[kDraftPoints];
    u32 n;
    return track_line(of, lane, t, &n, out) ? out[n - 1].step : ~0u;
}
static u32 at(u32 step) { return step * kStepTicks; }
static const u32 kSet = kAsSet << 8;
int main() {
    for (const Case& c : cases) {
        draft_of(c);
        require(store.put(store.count(), d), "every gesture's points are a valid variation");
    }
    require(store.size() == sizeof expected, "as many bytes as the tool writes");
    for (u32 i = 0; i < sizeof expected; ++i) require(store.bytes()[i] == expected[i], "the same bytes as the tool");

    u32 n;
    // Build: tracks 1-3 come in one after another, lowest first, from the bottom to as set.
    draft_of(cases[0]);
    u32 last = 0;
    for (u32 t = 0; t < 3; ++t) {
        const Point* p = line(0, t, &n);
        require(p && value_at(p, n, 0) == 0 && value_at(p, n, at(31)) == kSet, "build: from the bottom to as set");
        require(p[n - 1].step > last && p[n - 1].step % 4 == 0, "build: in turn, on the beat");
        last = p[n - 1].step;
        require(value_at(p, n, at(p[n - 1].step - 1)) == 0, "build: comes in suddenly");
    }
    require(!line(0, 3, &n), "build: other tracks untouched");
    // Lane b at once: its own tracks, a rise.
    for (u32 t = 3; t < 5; ++t) {
        const Point* p = line(1, t, &n);
        require(p && p[0].value == kAsSet && p[n - 1].value == 255 && p[n - 1].curve == kSlowStart,
                "rise (build 036): leaves as set, up to the top (full amount), its own slow start");
    }
    require(!line(1, 0, &n), "lane b: only its tracks");
    // Break, full amount: the highest track leaves first, the exits spread over the window.
    draft_of(cases[1]);
    u32 exits[4];
    for (u32 t = 0; t < 4; ++t) {
        const Point* p = line(0, t, &n);
        require(p && value_at(p, n, 0) == kSet && value_at(p, n, at(63)) == 0, "break: as set to the bottom, and stays");
        exits[t] = p[n - 1].step;
    }
    require(exits[3] < exits[2] && exits[2] < exits[1] && exits[1] < exits[0], "break: highest first");
    require(exits[3] <= 8 && exits[0] >= 52, "break: full amount spreads across the window");
    const Point* f = line(1, 5, &n);
    require(f && f[n - 1].value == kAsSet - 64 && f[n - 1].curve < 0, "fall: down by half, fast start");
    // No amount: all at once in the middle.
    draft_of(cases[2]);
    require(line(0, 7, &n) && line(0, 7, &n)[n - 1].step == 4, "break at 0: the middle");
    require(!d.lanes[1].count, "an empty lane: no points");
    // Drop: a beat to a bar before the end.
    draft_of(cases[3]);
    for (u32 t = 0; t < 8; ++t) require(line(0, t, &n) && line(0, t, &n)[n - 1].step == 32 - 10, "drop at 50%: 10 steps");
    // Swell and dip: out and back to as set by the window's end.
    const Point* s = line(1, 2, &n);
    require(s && n == 3 && s[1].step == 16 && s[1].value > 220 && s[2].value == kAsSet, "swell: up and back");
    require(value_at(s, n, at(31)) == kSet && value_at(s, n, at(16)) > value_at(s, n, at(8)), "swell: peaks in the middle");
    draft_of(cases[4]);
    const Point* dp = line(1, 6, &n);
    require(dp && n == 3 && dp[1].value == 0 && dp[2].value == kAsSet, "dip at full: to the bottom and back");
    require(line(0, 1, &n) && line(0, 1, &n)[n - 1].value == 0, "a drop on the notes: none");
    draft_of(cases[5]);
    require(!line(0, 2, &n), "swell at 0: nothing to do");
    // Curves: build's entries later with a slow start, break's exits earlier with a fast one.
    {
        Case c = cases[8];
        u32 slow[4], straight[4], fast[4];
        for (int which = 0; which < 3; ++which) {
            c.lanes[0].curve = which == 0 ? 90 : which == 1 ? 0 : -90;
            draft_of(c);
            for (u32 t = 0; t < 4; ++t) {
                const Point* p = line(0, t, &n);
                (which == 0 ? slow : which == 1 ? straight : fast)[t] = p[n - 1].step;
            }
        }
        u32 later = 0, earlier = 0;
        for (u32 t = 0; t < 4; ++t) {
            require(slow[t] >= straight[t] && fast[t] <= straight[t], "a slow start later, a fast one earlier");
            later += slow[t] > straight[t], earlier += fast[t] < straight[t];
            if (t) require(slow[t] >= slow[t - 1] && fast[t] >= fast[t - 1], "still lowest first (two may share a beat)");
        }
        require(later && earlier, "the curve moves them");
        c.lanes[0].curve = 0;
        draft_of(c);
        Case plain = c;
        plain.lanes[0].bent = false;
        Draft keep = d;
        draft_of(plain);
        for (u32 t = 0; t < 4; ++t) require(last_step(keep, 0, t) == last_step(d, 0, t), "build's own curve: straight, as before");
    }
    draft_of(cases[9]);
    {
        const Point* r = line(0, 0, &n);
        require(r && n == 2 && r[0].value == kAsSet && r[1].value == 255 && r[1].curve == -30, "rise: its own curve");
        const Point* w = line(1, 1, &n);
        require(w && n == 3 && w[1].curve == 64 && w[2].curve == -64, "swell: out by the curve, back by its mirror");
    }
    draft_of(cases[3]);
    {
        Case c = cases[3];
        c.lanes[0].bent = true, c.lanes[0].curve = 100;
        Draft keep = d;
        draft_of(c);
        for (u32 t = 0; t < 8; ++t) require(last_step(keep, 0, t) == last_step(d, 0, t), "drop: no curve");
        require(keep.lanes[0].count == d.lanes[0].count, "drop: the same line");
    }
    // Fade in: from silence (100 %) up to as set by the window's end, its own slow
    // start; fade out: from as set down by the amount, a fast start. 50 %: halfway.
    draft_of(cases[11]);
    {
        u32 fade_n;
        for (u32 t = 0; t < 2; ++t) {
            const Point* p = line(0, t, &fade_n);
            require(p && fade_n == 2 && p[0].step == 0 && p[0].value == 0 && p[1].step == 31 && p[1].value == kAsSet &&
                        p[1].curve == kSlowStart, "fade in: silence to as set, slow start");
            require(value_at(p, fade_n, at(8)) < value_at(p, fade_n, at(24)) && value_at(p, fade_n, at(31)) == kSet,
                    "fade in: rises to as set");
        }
        const Point* o = line(1, 2, &fade_n);
        require(o && fade_n == 2 && o[0].value == kAsSet && o[1].value == kAsSet - 64 && o[1].curve == kFastStart,
                "fade out at 50 %: as set to halfway down, fast start");
        require(natural_target(fade_in) == kVolume && natural_target(fade_out) == kVolume &&
                    default_curve(fade_in) == kSlowStart && default_curve(fade_out) == kFastStart,
                "the fades: the volume, rise's and fall's curves");
    }
    draft_of(cases[12]);
    {
        u32 fade_n;
        const Point* o = line(0, 3, &fade_n);
        require(o && o[1].value == 0 && o[1].curve == 30, "fade out at 100 %: to silence, the curve given");
        const Point* i = line(1, 4, &fade_n);
        require(i && i[0].value == kAsSet - 64 && i[1].value == kAsSet && i[1].curve == -50, "fade in at 50 %: from halfway");
        // A slow start stays lower early than a fast one.
        Case c = cases[12];
        c.lanes[0].curve = 100;
        draft_of(c);
        const u32 slow = value_at(line(0, 3, &fade_n), fade_n, at(4));
        c.lanes[0].curve = -100;
        draft_of(c);
        const u32 fast = value_at(line(0, 3, &fade_n), fade_n, at(4));
        require(slow > fast, "fade out: a slow start holds longer, a fast one drops early");
    }
    // Build 040: random, wander, cut in, gate.
    draft_of(cases[13]);
    {
        const DraftLane& r = d.lanes[0];
        // Sample and hold: the first value, then each beat a hold point (the value before) and a jump.
        require(r.count == 15 && r.points[0].step == 0 && r.points[14].step == 28, "random: a new value every beat");
        for (u32 k = 0; k < r.count; ++k) {
            const bool hold = k % 2 == 1, jump = k && !hold;
            require(r.points[k].step == (k + 1) / 2 * 4 - (hold ? 1 : 0), "random: holds on the step before each beat");
            require(!jump || r.points[k].curve == kSudden, "random: each new value a jump");
            require(!hold || r.points[k].value == r.points[k - 1].value, "random: each value held to the jump");
            require(r.points[k].value >= kAsSet - 99 && r.points[k].value <= kAsSet + 99, "random: within the amount's reach (200: 99)");
        }
        const DraftLane& w = d.lanes[1];
        require(w.count == 5 && w.points[0].value == kAsSet && w.points[4].step == 31 && w.points[4].value == kAsSet, "wander: from as set to as set");
        for (u32 k = 0; k < w.count; ++k) require(w.points[k].curve == 0 && (k == 4 || w.points[k].step == k * 8), "wander: a point every 2 beats, straight");
        // The same seed, the same points; a new seed, new ones.
        DraftLane again = d.lanes[0];
        generate(again, 32);
        require(again.count == r.count && !std::memcmp(again.points, r.points, r.count * sizeof(Point)), "random: the seed decides");
        bool differs = false;
        for (u32 seed = 1; seed < 5 && !differs; ++seed) {
            again.seed = seed;
            generate(again, 32);
            differs = std::memcmp(again.points, r.points, r.count * sizeof(Point)) != 0;
        }
        require(differs, "random: another seed, other values");
    }
    draft_of(cases[14]);
    require(d.lanes[0].count == 31 && d.lanes[1].count == 0, "random: 8 bars, every 2 beats: 31 points, within 32; wander at 0 has none");
    draft_of(cases[15]);
    {
        const DraftLane& c = d.lanes[0];  // 16 steps, 50 %: gap 10
        require(c.count == 3 && c.points[0].value == 0 && c.points[1].step == 9 && c.points[2].step == 10 && c.points[2].value == kAsSet &&
                    c.points[2].curve == kSudden, "cut in: the bottom, a jump to as set after the gap");
        const DraftLane& s = d.lanes[1];  // build 044: 16 steps at 100 %: on 0, off (silent) 4, on 8, off 12, jumps
        // on 0 (held to 3), off 4 (held to 7), on 8 (held to 11), off 12: hold points, then jumps
        const u32 steps[] = {0, 3, 4, 7, 8, 11, 12}, values[] = {128, 128, 0, 0, 128, 128, 0};
        require(s.count == 7, "gate: a beat on, a beat off over the bar, each held");
        for (u32 k = 0; k < s.count; ++k)
            require(s.points[k].step == steps[k] && s.points[k].value == values[k] && (k % 2 == 0 && k ? s.points[k].curve == kSudden : true),
                    "gate: held, then a jump at each beat");
        require(stutter_scale(16) == 1 && stutter_scale(32) == 2 && stutter_scale(128) == 8 && stutter_scale(8) == 1,
                "gate: its bar stretched over the window, repeated once a bar");
        require(natural_target(cut_in) == kVolume && natural_target(gate) == kVolume && natural_target(random_) == kCutoff &&
                    natural_target(wander) == kCutoff && default_curve(cut_in) == 0 && default_curve(wander) == 0, "the new targets and curves");
    }
    draft_of(cases[17]);
    {
        // Build 048: stutter is steady again (046's sound): its density line flat at the top.
        const DraftLane& s = d.lanes[0];
        require(s.gesture == stutter && s.gesture == 14 && s.count == 2 && s.points[0].value == 255 && s.points[1].value == 255 &&
                    s.points[1].step == 31,
                "stutter: flat at the top");
        // Build 047's line is stutter spring's: a slow-start climb to the top at three quarters of
        // the window, then a damped spring back to none (length 32: p 24, q 7).
        DraftLane sp;
        clear_lane(sp);
        sp.gesture = stutter_spring, sp.selection = 1, sp.amount = 255;
        generate(sp, 32);
        const u32 steps[6] = {0, 24, 25, 27, 29, 31}, values[6] = {128, 255, 170, 225, 150, 128};
        require(sp.count == 6, "stutter spring: six points");
        for (u32 k = 0; k < 6; ++k)
            require(sp.points[k].step == steps[k] && sp.points[k].value == values[k] && sp.points[k].curve == (k == 1 ? kSlowStart : 0),
                    "stutter spring: the density line");
        // Up climbs from none to the top, down falls from the top to none.
        sp.gesture = stutter_up;
        generate(sp, 32);
        require(sp.count == 2 && sp.points[0].value == 128 && sp.points[1].value == 255 && sp.points[1].curve == kSlowStart, "stutter up");
        sp.gesture = stutter_down;
        generate(sp, 32);
        require(sp.count == 2 && sp.points[0].value == 255 && sp.points[1].value == 128 && sp.points[1].curve == kFastStart, "stutter down");
        // Short windows leave out the points that would not be past the one before; the first and the peak stay.
        DraftLane t;
        clear_lane(t);
        t.gesture = stutter_spring, t.selection = 1, t.amount = 255;
        generate(t, 8);
        require(t.count == 3 && t.points[0].step == 0 && t.points[1].step == 6 && t.points[1].value == 255 && t.points[2].step == 7 &&
                    t.points[2].value == 128,
                "stutter spring at 8 steps: first, peak, last");
        t.selection = 0;
        generate(t, 32);
        require(t.count == 0, "stutter on no tracks: no line");
        require(natural_target(stutter) == kTargetNotes && default_curve(stutter) == 0 && gesture_ok(14) && gesture_ok(17) && gesture_ok(18) && !gesture_ok(19) &&
                    natural_target(stutter_down) == kTargetNotes,
                "stutter: on the notes, no curve, gestures 14-17 valid");
        require(gate == 13 && kGestureGate == 13, "gate keeps id 13");
    }
    draft_of(cases[16]);
    {
        const DraftLane& s = d.lanes[0];  // build 044: amount 0: no dip, no points
        require(s.count == 0, "gate at 0: nothing to dip");
        const DraftLane& c = d.lanes[1];  // 100 %: gap 16
        require(c.points[c.count - 1].step == 16 && c.points[c.count - 1].value == kAsSet, "cut in at 100 %: a bar in");
    }
    // Build 047: windows to 256 steps. No gesture's line passes 32 points, its steps stay within the window
    // (a point's step is a byte), and the dice's gestures put a point every 16 steps past 8 bars.
    for (u32 g = 1; g < kGestureCount; ++g)
        for (u32 length : {8u, 16u, 32u, 64u, 128u, 129u, 192u, 255u, 256u})
            for (u32 amount : {1u, 128u, 255u})
                for (u32 seed : {0u, 1u, 99u}) {
                    DraftLane l;
                    clear_lane(l);
                    l.gesture = g, l.amount = amount, l.selection = 0x07, l.curve = default_curve(g), l.seed = seed;
                    generate(l, length);
                    require(l.count <= kDraftPoints, "no line passes 32 points");
                    for (u32 k = 0; k < l.count; ++k)
                        require(l.points[k].step < length && (!k || l.points[k].step > l.points[k - 1].step), "points in step order within the window");
                }
    {
        DraftLane l;
        clear_lane(l);
        l.gesture = wander, l.amount = 200, l.selection = 1, l.seed = 5;
        generate(l, 256);
        require(l.count == 17 && l.points[1].step == 16 && l.points[15].step == 240 && l.points[16].step == 255, "wander at 256: a point every 16 steps");
        l.gesture = random_;
        generate(l, 256);
        require(l.count == 31 && l.points[1].step == 15 && l.points[2].step == 16, "random at 256: a new value every 16 steps (31 points)");
        generate(l, 128);
        require(l.points[1].step == 7 && l.points[2].step == 8, "random at 128: every 8");
        l.gesture = gate;
        generate(l, 256);
        require(l.count == 7 && l.points[6].step == 192 && stutter_scale(256) == 16, "gate at 256: its bar stretched 16 times");
    }
    // Version 5: one line a lane. Build and break: the line is the leading
    // track's, the stagger puts every track's entry or exit at turn_step(r),
    // where build 023 stored a jump a track (and so the store's upgrade of
    // those jumps, made again from the gesture, plays as they did).
    for (u32 g : {1u, 2u})
        for (u32 amount : {0u, 77u, 255u})
            for (int curve : {0, 90, -70})
                for (u32 sel : {0x01u, 0x81u, 0x5au, 0xffu})
                    for (u32 length : {8u, 32u, 128u, 256u}) {
                        DraftLane l;
                        clear_lane(l);
                        l.gesture = g, l.amount = amount, l.curve = curve, l.selection = sel;
                        generate(l, length);
                        const u32 tracks = count_bits(sel);
                        for (u32 t = 0; t < 8; ++t) {
                            if (!(sel >> t & 1)) continue;
                            const u32 r = stagger_rank(g, sel, t), turn = turn_step(r, tracks, amount, length, curve);
                            const u32 shift = stagger(g, amount, curve, sel, t, length);
                            for (u32 step = 0; step < length; ++step) {
                                const u32 v = value_late(l.points, l.count, step * kStepTicks, shift) >> 8;
                                const u32 want = g == 1 ? (step >= turn ? kAsSet : 0) : (step >= turn ? 0 : kAsSet);
                                require(v == want, "the stagger: each track's entry or exit where its own jump was");
                            }
                        }
                    }
    // Every other gesture: every track the same, no shift.
    for (u32 g = 0; g < kGestureCount; ++g)
        if (g != 1 && g != 2)
            for (u32 t = 0; t < 8; ++t) require(stagger(g, 255, 0, 0xff, t, 64) == 0, "no stagger but build and break");
    std::puts("variations gestures: ok");
    return 0;
}
