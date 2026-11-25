// Host test: stutter's note retrigger (retrigger.hpp), the pure maths (build 047:
// the lane's line the density) and the lane lookup.
#include "../gestures.hpp"
#include "../retrigger.hpp"
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
static Draft d;
// A line of (step, value) pairs as store points (no curves), for retrigger_plan.
static Point line[8];
static Stutter lane(u32 rate, u32 amount, u32 at, const u32* pairs, u32 n) {
    for (u32 k = 0; k < n; ++k) line[k] = Point{static_cast<u8>(pairs[2 * k]), static_cast<u8>(pairs[2 * k + 1]), 0};
    return Stutter{rate, amount, 0, at, n, 0, n ? line : nullptr};
}
int main() {
    // Levels: log2(rate) + 1; a value's level is the share of the way from as set to the top, rounded up.
    require(retrigger_levels(1) == 1 && retrigger_levels(2) == 2 && retrigger_levels(4) == 3 && retrigger_levels(8) == 4, "levels by rate");
    require(retrigger_level(128 << 8, 4) == 0 && retrigger_level(100 << 8, 4) == 0 && retrigger_level((128 << 8) + 1, 4) == 1 &&
                retrigger_level(255 << 8, 4) == 4 && retrigger_level(255 << 8, 1) == 1 && retrigger_level(191 << 8, 2) == 1 &&
                retrigger_level(192 << 8, 2) == 2,
            "level of a value");
    // The interval: a beat at level 1, halving each level up to half a step at 4.
    require(retrigger_interval(1) == 1920 && retrigger_interval(2) == 960 && retrigger_interval(3) == 480 && retrigger_interval(4) == 240,
            "interval by level");
    Repeat out[kMaxRepeats];
    // A flat top line at 1x: a repeat each beat in the room (build 046's behaviour), velocity falling with time.
    {
        const u32 flat[4] = {0, 255, 15, 255};
        const Stutter s = lane(1, 255, 0, flat, 2);
        require(retrigger_plan(s, 100, 5000, 1920, out) == 0 && retrigger_plan(s, 100, 5000, 1921, out) == 1 && out[0].delay == 1920,
                "1x: the first beat after the note, only if room remains");
        const u32 n = retrigger_plan(s, 100, 5000, 1920 * 3 + 1, out);
        require(n == 3 && out[0].delay == 1920 && out[1].delay == 3840 && out[2].delay == 5760, "1x: a beat apart");
        require(out[0].length == 1920 && retrigger_plan(s, 100, 500, 1920 * 3 + 1, out) == 3 && out[0].length == 500, "a repeat's length: the note's or the interval");
        require(out[0].velocity == 100 * (255 - 255 * 1920 / 5761) / 255 && out[2].velocity < out[0].velocity, "velocity falls with time");
        require(retrigger_plan(lane(1, 0, 0, flat, 2), 100, 5000, 5761, out) == 3 && out[2].velocity == 100, "no fade at 0 %");
        require(retrigger_plan(lane(1, 255, 0, flat, 2), 0, 5000, 100000, out) == kMaxRepeats && out[kMaxRepeats - 1].velocity >= 1,
                "at most 16, velocity at least 1");
    }
    // A 128 line: no repeats at all.
    {
        const u32 none[4] = {0, 128, 15, 128};
        require(retrigger_plan(lane(4, 255, 0, none, 2), 100, 480, 61440, out) == 0, "as set: none");
    }
    // No points (a lane saved by build 046): full density, at the rate's speed.
    require(retrigger_plan(lane(4, 128, 0, nullptr, 0), 100, 5000, 1441, out) == 3 && out[0].delay == 480 && out[2].delay == 1440 &&
                retrigger_plan(lane(8, 128, 0, nullptr, 0), 100, 5000, 100000, out) == kMaxRepeats && out[0].delay == 240,
            "no points: the rate's speed");
    // A ramp from as set up to the top at 4x: the intervals shrink (a beat, a half beat, a step) as the line climbs.
    {
        const u32 ramp[4] = {0, 128, 15, 255};
        const u32 n = retrigger_plan(lane(4, 128, 0, ramp, 2), 100, 5000, 7680, out);
        require(n >= 5, "a ramp: several repeats");
        u32 shrinking = 1;
        for (u32 k = 2; k < n; ++k) shrinking &= out[k].delay - out[k - 1].delay <= out[k - 1].delay - (k > 1 ? out[k - 2].delay : 0);
        require(shrinking && out[n - 1].delay - out[n - 2].delay == 480 && out[1].delay - out[0].delay >= 480, "a ramp: shrinking intervals");
        require(out[0].delay >= 480, "a ramp: nothing at the line's start");
    }
    // The line is read at the window time of the note: the same note later in the window repeats sooner.
    {
        const u32 ramp[4] = {0, 128, 15, 255};
        require(retrigger_plan(lane(4, 128, 0, ramp, 2), 100, 5000, 1920, out) <= 1, "early in the ramp: slow or none");
        const u32 n = retrigger_plan(lane(4, 128, 15 * 480, ramp, 2), 100, 5000, 1920, out);
        require(n == 3 && out[0].delay == 480, "at the top: a step apart");
    }
    // The lane lookup: a stutter lane on track 2 (selection 0x02), 16 steps from the join, rate 4, 50 %.
    d.song = 0, d.entry = 1, d.offset = 0, d.length = 16;
    d.unplaced = false, d.scene_before = d.scene_after = kNoScene;
    for (DraftLane& l : d.lanes) clear_lane(l);
    d.lanes[0].gesture = stutter, d.lanes[0].amount = 128, d.lanes[0].selection = 0x02, d.lanes[0].rate = 4;
    d.lanes[0].target = kTargetNotes;
    generate(d);
    require(store.put(0, d), "a stutter lane stores");
    SongPosition s = {};
    s.playing = true, s.song = 0, s.entry = 1, s.next_entry = 2, s.position = 480 * 4, s.length = 480 * 64;
    s.scene = kNoScene, s.scene_before = kNoScene;
    Stutter st;
    require(find_stutter(store, s, 1, &st) && st.rate == 4 && st.amount == 128 && st.remaining == 480 * 12, "found: rate, amount, time left");
    require(st.at == 480 * 4 && st.count == 2 && st.points && st.points[1].value == 255, "found: the window time and the line (build 048: steady, flat at the top)");
    require(!find_stutter(store, s, 0, &st), "not on a track outside the lane");
    {
        // Build 047: a muted stutter lane retriggers nothing.
        Draft m = d;
        m.lanes[0].muted = true;
        require(store.put(0, m) && !find_stutter(store, s, 1, &st), "a muted stutter lane: not found");
        require(store.put(0, d) && find_stutter(store, s, 1, &st), "unmuted: found again");
    }
    {
        // Build 060 (version 11): a stutter in lane C is found as in lane A, on its own tracks, with lanes A and B empty.
        Draft c = d;
        c.lanes[2] = c.lanes[0], c.lanes[2].selection = 0x20, c.lanes[2].rate = 8, c.lanes[2].amount = 77;
        clear_lane(c.lanes[0]);
        require(store.put(0, c), "a stutter in lane C stores");
        require(find_stutter(store, s, 5, &st) && st.rate == 8 && st.amount == 77 && st.at == 480 * 4 && st.count == 2,
                "lane C's stutter found on its track (track 6)");
        require(!find_stutter(store, s, 1, &st), "and not on lane A's old track");
        require(store.put(0, d), "restored");
    }
    s.position = 480 * 16;
    require(!find_stutter(store, s, 1, &st), "not past the window's end");
    s.entry = 0;
    require(!find_stutter(store, s, 1, &st), "not in another entry");
    std::puts("variations retrigger: ok");
    return 0;
}
