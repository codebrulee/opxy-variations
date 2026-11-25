// Host test: build 049's long windows (format version 9): a window up to 1024
// steps whose line has its own length, line_length() steps (at most 256), and is
// stretched over it (stagger.hpp value_late). The gestures draw over the line;
// the gate stays a real beat; the drop's gap stays about real time.
#include "../gestures.hpp"
#include "../retrigger.hpp"
#include "../window.hpp"
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
// One variation of `length` steps on entry 1, lane a a `gesture` of `amount` on track 1's volume.
static void make(u32 length, u32 gesture, u32 amount, int curve, u32 selection = 1) {
    Draft d = {};
    d.song = 0, d.entry = 1, d.offset = 0, d.length = length;
    d.scene_before = d.scene_after = kNoScene;
    for (u32 n = 0; n < kLanes; ++n) clear_lane(d.lanes[n]);
    DraftLane& l = d.lanes[0];
    l.gesture = gesture, l.amount = amount, l.target = kVolume, l.selection = selection, l.curve = curve, l.rate = 1;
    generate(d);
    store.clear();
    require(store.put(0, d), "put");
}
// The track's volume asked at the middle of window step `step` (Q8), or -1.
static i32 asked(u32 step, u32 track = 0) {
    SongPosition s = SongPosition{true, 0, 1, 2, step * 480 + 240, 4000 * 480, 0, kNoScene};
    Targets tg;
    targets(store, s, &tg);
    u32 value = 0;
    return tg.find(track, kVolume, &value) ? static_cast<i32>(value) : -1;
}
int main() {
    require(line_length(8) == 8 && line_length(256) == 256 && line_length(512) == 256 && line_length(1024) == 256, "line_length");
    // A rise (straight) over 1024 steps: halfway at step 512, its top at the window's end.
    make(1024, rise, 255, 0);
    require(store[0].length == 1024 && store[0].lanes[0].count == 2, "a rise over 1024 steps: two points");
    u32 n = 0;
    const Point* p = store.points(store[0], 0, &n);
    require(n == 2 && p[1].step == 255 && p[1].value == 255, "the line's last point is step 255, the line's end");
    const i32 mid = asked(512), end = asked(1023), start = asked(0);
    require(start >= 128 * 256 && start < 129 * 256, "a rise starts as set");
    require(mid > 190 * 256 && mid < 194 * 256, "halfway up at step 512 (the stretch)");
    require(end > 254 * 256 + 128, "at its top by the window's end");
    require(asked(256) > 159 * 256 && asked(256) < 161 * 256, "a quarter up at step 256");
    // A window of 256 steps or fewer is not stretched: a 200-step rise is halfway at step 100.
    make(200, rise, 255, 0);
    require(asked(100) > 190 * 256 && asked(100) < 194 * 256, "no stretch up to 256 steps");
    // 512 steps: stretched by two.
    make(512, rise, 255, 0);
    require(asked(256) > 190 * 256 && asked(256) < 194 * 256, "a 512-step window: halfway at 256");
    // The gate: a real beat on, a beat off (4 steps each) at 1024 steps as at 16; each repeat one bar.
    make(1024, gate, 255, 0);
    require(store[0].lanes[0].count == 7, "a gate: one bar, 7 points");
    p = store.points(store[0], 0, &n);
    require(p[6].step == 192, "the gate bar is stored over the 256-step line");
    for (u32 s = 0; s < 1024; s += 1) {
        const bool on = (s % 16) / 4 % 2 == 0;
        const i32 v = asked(s);
        require(on ? v == 128 * 256 : v == 0, "gate at 1024: on for 4 steps, off for 4, in window steps");
    }
    // Drop's gap is about real time: 4..16 window steps, 1..4 line steps at 1024.
    require(drop_gap(0, 1024) == 1 && drop_gap(255, 1024) == 4 && drop_gap(0, 512) == 2 && drop_gap(255, 512) == 8, "drop gap in line steps");
    require(drop_gap(0, 256) == 4 && drop_gap(255, 256) == 16 && drop_gap(0, 16) == 4, "drop gap unchanged to 256 steps");
    make(1024, drop, 0, 0);
    p = store.points(store[0], 0, &n);
    require(n == 3 && p[1].step == 254 && p[2].step == 255 && p[2].value == 0, "drop at 1024: one line step, the last");
    require(asked(1015) == 128 * 256 && asked(1021) == 0, "the drop falls within the last 4 window steps (one line step)");
    make(1024, drop, 255, 0);
    require(asked(1003) == 128 * 256 && asked(1021) == 0, "a full drop falls over the last 16 window steps (4 line steps)");
    // Build: the stagger is spread over the line's length, so the second track follows the first in line steps.
    require(stagger(kGestureBuild, 255, 0, 3, 1, 1024) == stagger(kGestureBuild, 255, 0, 3, 1, 256) && stagger(kGestureBuild, 255, 0, 3, 1, 1024) > 0,
            "the stagger is in line steps");
    make(1024, build, 255, 0, 3);
    const i32 first = asked(1023, 0), second = asked(1023, 1);
    require(first == 128 * 256 && second == 128 * 256, "a build at 1024: both tracks in by the window's end");
    require(asked(0, 0) == 0 && asked(0, 1) == 0, "a build at 1024: both start silent");
    // Random and wander stay within 32 points on any length.
    make(1024, random_, 255, 0);
    require(store[0].lanes[0].count <= kDraftPoints && store[0].lanes[0].count > 4, "random over 1024: within 32 points");
    make(1024, wander, 255, 0);
    require(store[0].lanes[0].count <= kDraftPoints, "wander over 1024: within 32 points");
    // Stutter's density line is read stretched too: a stutter up reaches full density by the end of 1024.
    {
        make(1024, stutter_up, 255, 0);
        Stutter s = {};
        SongPosition at = SongPosition{true, 0, 1, 2, 1000 * 480, 4000 * 480, 0, kNoScene};
        require(find_stutter(store, at, 0, &s) && s.length == 1024, "stutter carries the window's length");
        require(retrigger_level(value_late(s.points, s.count, 1000 * 480, 0, 1, s.length), 1) == 1, "dense late in the window");
    }
    // Build 063 (version 12): windows to 4096 steps (256 bars). The line is still 256 steps, stretched by 16.
    {
        require(line_length(2048) == 256 && line_length(4096) == 256, "line_length at 2048 and 4096");
        make(4096, rise, 255, 0);
        require(store[0].length == 4096 && store[0].lanes[0].count == 2, "a rise over 4096 steps: two points");
        require(asked(2048) > 190 * 256 && asked(2048) < 194 * 256, "halfway up at step 2048 (the stretch)");
        require(asked(4095) > 254 * 256 + 128 && asked(0) < 129 * 256, "its top at the end, as set at the start");
        // The gate still plays a real beat on and off over 4096 steps: 256 repeats of its bar.
        make(4096, gate, 255, 0);
        for (u32 s = 0; s < 4096; s += 3) {
            const bool on = (s % 16) / 4 % 2 == 0;
            const i32 v = asked(s);
            require(on ? v == 128 * 256 : v == 0, "gate at 4096: on for 4 steps, off for 4, in window steps");
        }
        // value_late's rate step: the busiest read, a gate at 8x over 4096 steps (rate 8 * 256), at the window's last tick, is
        // the same as a 64-bit calculation would give (at % span * rate fits a u32 where at * rate nearly did not).
        make(4096, gate, 255, 0);
        u32 count = 0;
        const Point* line = store.points(store[0], 0, &count);
        const u32 span = 4096 * 480;
        for (u32 at = span - 1; at + 200000 > span - 1 && at > 0; at -= 7919) {
            const unsigned long long mapped = static_cast<unsigned long long>(at) * 2048 % span;
            require(value_late(line, count, at, 0, 2048, 4096) == value_late(line, count, static_cast<u32>(mapped / 16), 0),
                    "value_late: rate 2048 over 4096 steps reads the same as a 64-bit mapping");
        }
        // An ease's tail, a little past the span: still no overflow.
        const unsigned long long tail = static_cast<unsigned long long>(span + 300000) * 2048 % span;
        require(value_late(line, count, span + 300000, 0, 2048, 4096) == value_late(line, count, static_cast<u32>(tail / 16), 0),
                "value_late: past the span, the same");
    }
    std::puts("long windows ok");
    return 0;
}
