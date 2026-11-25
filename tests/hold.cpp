// Host test, build 062 ("it might be good to have a flat line gesture", "also add in the flat line /
// silence curve please"): the hold gesture's line, what it drives, and its store round trip. Its name on the
// strip is in tests/page.cpp, the tool's side in tests/test_variations_tool.py.
#include "../gestures.hpp"
#include "../window.hpp"
#include <initializer_list>
#include <cstdio>
#include <cstdlib>
using namespace variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static Store store;
static SongPosition at(u32 step) { return SongPosition{true, 0, 1, 2, step * 480, 64 * 480, 0, kNoScene}; }
static void hold_draft(Draft& d, u32 length, u32 amount, u32 target, u32 selection) {
    d = Draft();
    d.song = 0, d.entry = 1, d.offset = 0, d.length = length;
    d.unplaced = false, d.scene_before = d.scene_after = kNoScene;
    for (DraftLane& l : d.lanes) clear_lane(l);
    DraftLane& l = d.lanes[0];
    l.gesture = hold, l.amount = amount, l.target = target, l.selection = selection;
    l.default_tracks = false, l.default_target = target == kVolume, l.curve = default_curve(hold);
    generate(d);
}
int main() {
    require(hold == 18 && stutter_down == 17 && kGestureCount == 19 && gesture_ok(18) && !gesture_ok(19), "hold is gesture 18, after stutter out");
    require(default_curve(hold) == 0 && natural_target(hold) == kVolume && !starts_on_volume(hold), "hold: no bend, the volume to start, any target");
    // A flat line at 0 on every window length, the shortest (8) included; the line's last step is length - 1 up to
    // 256 steps and 255 past it (build 049).
    for (u32 length : {8u, 16u, 32u, 64u, 128u, 256u, 512u, 1024u}) {
        DraftLane l;
        clear_lane(l);
        l.gesture = hold, l.amount = 255, l.selection = 1;
        generate(l, length);
        const u32 last = (length < 256 ? length : 256) - 1;
        require(l.count == 2 && l.points[0].step == 0 && l.points[1].step == last, "hold: first and last step");
        require(l.points[0].value == 0 && l.points[1].value == 0 && l.points[0].curve == 0 && l.points[1].curve == 0, "hold: flat at 0, straight");
    }
    // Degenerate: a one-step window (not offered, 8 is the shortest) is one point, never two at a step.
    {
        DraftLane l;
        clear_lane(l);
        l.gesture = hold, l.amount = 255, l.selection = 1;
        generate(l, 1);
        require(l.count == 1 && l.points[0].step == 0 && l.points[0].value == 0, "hold on one step: one point");
        l.selection = 0;
        generate(l, 32);
        require(l.count == 0, "hold on no tracks: no line");
    }
    // Part way: the amount scales the depth as the fades' does; 0 leaves as set (no line).
    {
        DraftLane l;
        clear_lane(l);
        l.gesture = hold, l.selection = 1;
        l.amount = 128;
        generate(l, 32);
        require(l.count == 2 && l.points[0].value == l.points[1].value && l.points[0].value == kAsSet - 64, "hold at 50 %: halfway down");
        l.amount = 0;
        generate(l, 32);
        require(l.count == 0, "hold at 0 %: as set, no points");
    }
    // What it drives: silence on the volume across the whole window at full amount, part way at less; a muted
    // lane and a zero amount drive nothing; the eases and the window's edges.
    Draft d;
    Targets tg;
    u32 value;
    hold_draft(d, 32, 255, kVolume, 0x05);
    require(store.put(store.count(), d), "stored");
    for (u32 step : {0u, 1u, 15u, 31u}) {
        targets(store, at(step), &tg);
        require(tg.count == 2 && tg.find(0, kVolume, &value) && value == 0 && tg.find(2, kVolume, &value) && value == 0, "hold: silence all the way");
    }
    targets(store, at(32), &tg);
    require(tg.count == 0, "hold: over at the window's end");
    // The eases: before the window the value comes down from as set, after it back up (never past either).
    targets(store, at(0), &tg, 4);
    require(tg.find(0, kVolume, &value) && value == 0, "hold: inside the window the ease leaves it alone");
    {
        const Variation& v = store[0];
        require(v.lanes[0].gesture == hold && v.lanes[0].amount == 255 && v.lanes[0].curve == 0, "round trip: gesture, amount, curve");
        u32 n;
        const Point* p = store.points(v, 0, &n);
        require(p && n == 2 && p[0].step == 0 && p[1].step == 31 && p[0].value == 0 && p[1].value == 0, "round trip: the two points");
        // the bytes load back to the same bytes
        static u8 copy[4096];
        const u32 size = store.size();
        for (u32 i = 0; i < size; ++i) copy[i] = store.bytes()[i];
        Store other;
        require(other.load(copy, size) && other.size() == size && other.count() == 1 && other[0].lanes[0].gesture == hold, "the bytes load again");
        for (u32 i = 0; i < size; ++i) require(other.bytes()[i] == copy[i], "and save the same bytes");
        u32 m;
        const Point* q = other.points(other[0], 0, &m);
        require(q && m == 2 && q[1].step == 31 && q[1].value == 0, "the points survive the load");
    }
    // Part way on the volume: halfway down at 50 %.
    {
        Store half;
        hold_draft(d, 32, 128, kVolume, 0x01);
        require(half.put(0, d), "stored");
        targets(half, at(10), &tg);
        require(tg.find(0, kVolume, &value) && value == (kAsSet - 64) << 8, "hold at 50 %: halfway down the volume");
    }
    // A zero amount drives nothing; a muted lane plays nothing.
    {
        Store zero;
        hold_draft(d, 32, 0, kVolume, 0x01);
        require(zero.put(0, d), "stored");
        targets(zero, at(10), &tg);
        require(tg.count == 0, "hold at 0 %: drives nothing, the volume stays as set");
        Store muted;
        hold_draft(d, 32, 255, kVolume, 0x01);
        d.lanes[0].muted = true;
        require(muted.put(0, d), "stored");
        targets(muted, at(10), &tg);
        require(tg.count == 0, "hold muted: plays nothing");
    }
    // Any target: the cutoff too.
    {
        Store cutoff;
        hold_draft(d, 16, 255, kCutoff, 0x02);
        require(cutoff.put(0, d), "stored");
        targets(cutoff, at(5), &tg);
        require(tg.find(1, kCutoff, &value) && value == 0 && !tg.find(1, kVolume, &value), "hold on the cutoff: the bottom of it");
    }
    return 0;
}
