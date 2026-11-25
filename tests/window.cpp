// Host test: window.hpp places windows from the song position, and what the
// lanes ask of each track there.
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
static u8 bytes[256];
static u32 size;
static void put(u32 b) { bytes[size++] = static_cast<u8>(b); }
// One variation: lane a on track 1's volume, 0 at its first step, 255 at its
// last; lane b empty, or (with `b`) a flat 200 on track 2's cutoff.
static void add(u32 song, u32 entry, int offset, u32 length, bool b = false) {
    put(song); put(entry); put(offset & 0xff); put((offset >> 8) & 0xff); put(length); put(0);
    put(0); put(kNoScene); put(kNoScene);
    put(0); put(0); put(kVolume); put(1); put(0); put(1);
    put(0); put(2);
    put(0); put(0); put(0);
    put(length - 1); put(255); put(0);
    if (!b) {
        put(kNoGesture); put(0); put(kNoTarget); put(0); put(0); put(0);
        return;
    }
    put(0); put(0); put(kCutoff); put(2); put(0); put(1);
    put(1); put(1); put(0); put(200); put(0);
}
static void load(u32 count) {
    bytes[0] = 'V'; bytes[1] = 'R'; bytes[2] = 3; bytes[3] = static_cast<u8>(count);
    require(store.load(bytes, size), "loads");
}
static SongPosition at(u32 song, u32 entry, u32 step, u32 length_steps) {
    return SongPosition{true, song, entry, entry + 1, step * 480, length_steps * 480, 0, kNoScene};
}
int main() {
    size = 4;
    add(0, 2, -32, 32);  // ends on the join into entry 3 (index 2)
    add(0, 4, -8, 16);   // straddles the join into entry 5
    add(0, 1, 24, 16);   // inside entry 2, steps 24-39
    load(3);
    u32 a;
    // Ending on a join: entry 2 (index 1) of 64 steps, window in its last 32.
    require(!window_at(store[0], at(0, 1, 31, 64), &a), "not before the window");
    require(window_at(store[0], at(0, 1, 32, 64), &a) && a == 0, "starts 32 steps before the join");
    require(window_at(store[0], at(0, 1, 63, 64), &a) && a == 31 * 480, "last step before the join");
    require(!window_at(store[0], at(0, 2, 0, 64), &a), "over at the join");
    // A short section before the join: starts part-way.
    require(window_at(store[0], at(0, 1, 0, 16), &a) && a == 16 * 480, "a 16-step section starts it halfway");
    // Straddling: 8 steps before the join into entry 5, 8 after.
    require(window_at(store[1], at(0, 3, 56, 64), &a) && a == 0, "straddle: starts before the join");
    require(window_at(store[1], at(0, 4, 0, 64), &a) && a == 8 * 480, "straddle: carries on after it");
    require(window_at(store[1], at(0, 4, 7, 64), &a) && a == 15 * 480, "straddle: last step");
    require(!window_at(store[1], at(0, 4, 8, 64), &a), "straddle: over");
    // Inside a section.
    require(!window_at(store[2], at(0, 1, 23, 64), &a), "inside: not yet");
    require(window_at(store[2], at(0, 1, 24, 64), &a) && a == 0, "inside: starts");
    require(!window_at(store[2], at(0, 0, 63, 64), &a), "inside: never from the section before");
    // Another song, or no song playing.
    require(!window_at(store[0], at(1, 1, 40, 64), &a), "other song");
    SongPosition off = at(0, 1, 40, 64);
    off.playing = false;
    require(!window_at(store[0], off, &a), "not playing");
    // Targets: the lane's value; overlaps go to the first listed.
    Targets tg;
    u32 value;
    targets(store, at(0, 1, 48, 64), &tg);
    require(tg.find(0, kVolume, &value) && value > 120 * 256 && value < 135 * 256, "halfway up");
    require(tg.count == 1 && !tg.find(1, kVolume, &value) && !tg.notes_set[0], "only the tracks with points");
    targets(store, at(0, 1, 28, 64), &tg);
    require(tg.find(0, kVolume, &value) && value > 60 * 256 && value < 75 * 256,
            "variation 3 alone at step 28 (4 of 15)");
    targets(store, at(0, 1, 36, 64), &tg);
    require(tg.count == 1 && tg.find(0, kVolume, &value) && value > 30 * 256 && value < 36 * 256,
            "step 36: both play, the first listed decides (4 of 31, not 12 of 15)");
    // Two lanes: each its own tracks and target.
    size = 4;
    add(0, 2, -32, 32, true);
    // A second variation on the same window: a notes lane on track 3.
    put(0); put(2); put(0xe0); put(0xff); put(32); put(0); put(0); put(kNoScene); put(kNoScene);
    put(0); put(0); put(kTargetNotes); put(4); put(0); put(1);
    put(2); put(1); put(0); put(64); put(0);
    put(kNoGesture); put(0); put(kNoTarget); put(0); put(0); put(0);
    load(2);
    targets(store, at(0, 1, 63, 64), &tg);
    require(tg.count == 2, "two words: track 1's volume and track 2's cutoff");
    require(tg.find(0, kVolume, &value) && value == 255 << 8, "track 1's volume: lane a's");
    require(tg.find(1, kCutoff, &value) && value == 200 << 8, "track 2's cutoff: lane b's");
    require(!tg.find(0, kCutoff, &value) && !tg.find(1, kVolume, &value), "nothing else");
    require(tg.notes_set[2] && tg.notes[2] == 64 << 8 && !tg.notes_set[0], "track 3's notes, from the second variation");
    // Build 041: a lane's rate. The line (0 at the first step, 255 at the last) plays `rate` times across
    // the window: the same value at t and at t + span / rate, rate 1 as before.
    {
        Draft d;
        size = 4;
        add(0, 2, -32, 32);
        load(1);
        store.draft(0, d);
        const u32 span = 32 * kStepTicks;
        u32 base[8], at_rate[4][8];
        const u32 rates[4] = {1, 2, 4, 8};
        for (u32 r = 0; r < 4; ++r) {
            d.lanes[0].rate = rates[r];
            require(store.put(0, d) && store[0].lanes[0].rate == rates[r], "rate stored");
            for (u32 k = 0; k < 8; ++k) {
                SongPosition p = at(0, 1, 32, 64);
                p.position += k * span / 8 + 7;
                targets(store, p, &tg);
                require(tg.find(0, kVolume, &value), "driven");
                at_rate[r][k] = value;
                if (r == 0) base[k] = value;
            }
        }
        for (u32 k = 0; k < 8; ++k) require(at_rate[0][k] == base[k], "rate 1: as before");
        for (u32 k = 0; k < 4; ++k) require(at_rate[1][k] == at_rate[1][k + 4], "rate 2: the same at t and t + span / 2");
        for (u32 k = 0; k < 4; ++k) require(at_rate[1][k] != base[k] || k == 0, "rate 2: not the slow line");
        require(at_rate[2][0] == at_rate[2][2] && at_rate[2][1] == at_rate[2][3] && at_rate[2][0] == at_rate[2][4], "rate 4: every span / 4");
        require(at_rate[3][0] == at_rate[3][1] && at_rate[3][2] == at_rate[3][5], "rate 8: every span / 8");
        // At rate 2 the second cycle starts at 0 again, one tick after the window's middle reads the line's start.
        SongPosition mid = at(0, 1, 32 + 16, 64);
        d.lanes[0].rate = 2;
        require(store.put(0, d), "rate 2 again");
        targets(store, mid, &tg);
        require(tg.find(0, kVolume, &value) && value == 0, "rate 2: the second cycle starts from the line's start");
        mid.position -= 1;
        targets(store, mid, &tg);
        require(tg.find(0, kVolume, &value) && value > 250 * 256, "rate 2: the first cycle ends at the line's end");
        // The window's outside is not mapped (the eases, window_time).
        SongPosition before = at(0, 1, 31, 64);
        before.position = 64 * 480 - 32 * 480 - 20;  // 20 ticks before the window starts
        targets(store, before, &tg, 40);
        require(tg.find(0, kVolume, &value) && value == 64 * 256, "the lead-in ease is as it was at rate 2");
    }
    // A looping song: the last entry's next join starts entry 1, so a window
    // before entry 1's join plays at the end of the last entry.
    size = 4;
    add(0, 0, -16, 16);
    load(1);
    SongPosition last = at(0, 5, 56, 64);
    last.next_entry = 0;
    require(window_at(store[0], last, &a) && a == 8 * 480, "a looping song's last entry leads into entry 1");
    last.next_entry = 6;
    require(!window_at(store[0], last, &a), "not when another entry follows");
    // Scenes known: where playback (re)started at the entry (Play, a jump, the
    // song's start) the scene it came from can't be asked of a window that
    // begins at or after the join; one that began before the join still must
    // have come from its scene before.
    size = 4;
    add(0, 0, 0, 16);   // on the song's start
    add(0, 0, -8, 16);  // straddling it
    load(2);
    for (u32 i = 0; i < 2; ++i) store.pin(i, 0, 2, 0, false);  // scene 3 > scene 1
    SongPosition start = at(0, 0, 4, 64);
    start.scene = 0, start.scene_before = 5, start.restarted = true;  // came round from scene 6 (the song's end edited)
    require(window_at(store[0], start, &a), "restarted at the song's start: plays, whatever came before");
    require(!window_at(store[1], start, &a), "straddling, from another scene: silent");
    start.restarted = false;
    require(!window_at(store[0], start, &a), "stepped in from another scene mid-song: silent");
    start.scene_before = kNoScene;
    require(window_at(store[0], start, &a) && window_at(store[1], start, &a), "Play (nothing came before): both play");
    start.scene = 1;
    start.restarted = true;
    require(!window_at(store[0], start, &a), "the scene after must still be its own");
    // The last entry of a song that doesn't loop: no join follows it.
    SongPosition end = at(0, 2, 60, 64);
    end.next_entry = 0, end.scene = 2;
    require(window_at(store[1], end, &a), "looping: the straddling window starts in the last entry");
    end.stops = true;
    require(!window_at(store[1], end, &a), "not looping: nothing before a join that never comes");
    // ahead(): where the next pass will be, this pass's advance on (values lead by a pass).
    {
        SongPosition p = at(0, 1, 0, 64);
        p.scene = 1, p.position = 64 * 480 - 10;  // 10 ticks before the join into entry 3
        const SongPosition n = ahead(p, 20);
        require(n.entry == 2 && n.position == 10 && n.scene == kNoScene && n.scene_before == 1 && !n.restarted &&
                    n.next_entry == kNoEntry && !n.stops,
                "over the join: the next entry from its start, its scene not known yet, come from this one");
        require(ahead(p, 5).entry == 1 && ahead(p, 5).position == p.position + 5, "within the scene: on by the pace");
        require(ahead(p, 0).position == p.position, "no pace yet: no lead");
        require(ahead(p, 20, 7).scene == 7, "a cued scene: that one");
        p.next_entry = 0;
        require(ahead(p, 20).entry == 0 && ahead(p, 20).restarted, "round to the song's start: a restart, as the pass finds it");
        p.stops = true;
        require(ahead(p, 20).entry == 1 && ahead(p, 20).position == p.position, "the song stops there: nothing ahead");
        SongPosition idle = p;
        idle.playing = false;
        require(ahead(idle, 20).position == p.position, "not playing: as it is");
        // The scene guard over the join: the straddling window (scenes 4 > 5 pinned) plays
        // ahead from the scene before; a cued other scene, or another scene before, stays silent.
        require(store.pin(1, 4, 3, 4, false), "pinned to scenes 4 > 5");
        SongPosition q = at(0, 3, 0, 64);
        q.position = 64 * 480 - 4, q.scene = 3;
        require(window_at(store[1], ahead(q, 20), &a) && a == 8 * 480 + 16, "ahead over the join: its window, 16 in");
        require(!window_at(store[1], ahead(q, 20, 9), &a), "a cued other scene: silent");
        q.scene = 5;
        require(!window_at(store[1], ahead(q, 20), &a), "come from another scene: silent");
    }
    // Build 024 (format version 5): a window wholly in the section before its join, as far
    // back as that section's start, plays only there, still pinned to (and guarded by) its join.
    {
        Store s;
        Draft d = {};
        d.song = 0, d.entry = 2, d.offset = -64, d.length = 16;
        d.scene_before = d.scene_after = kNoScene;
        clear_lane(d.lanes[0]), clear_lane(d.lanes[1]), clear_lane(d.lanes[2]);
        d.lanes[0].gesture = 0, d.lanes[0].target = kVolume, d.lanes[0].selection = 1;
        d.lanes[0].count = 2, d.lanes[0].points[0] = Point{0, 0, 0}, d.lanes[0].points[1] = Point{15, 255, 0};
        require(s.put(0, d), "a far-left window is a valid variation");
        u32 w;
        require(window_at(s[0], at(0, 1, 0, 64), &w) && w == 0, "far left: from the section before's start");
        require(window_at(s[0], at(0, 1, 15, 64), &w) && w == 15 * 480, "its last step, 48 before the join");
        require(!window_at(s[0], at(0, 1, 16, 64), &w) && !window_at(s[0], at(0, 1, 63, 64), &w), "over before the join");
        require(!window_at(s[0], at(0, 2, 0, 64), &w) && !window_at(s[0], at(0, 2, 10, 64), &w), "never after it");
        require(!window_at(s[0], at(0, 1, 0, 32), &w), "a section before shorter than its offset: not reached");
        Targets t;
        targets(s, at(0, 1, 15, 64), &t);
        u32 v;
        require(t.find(0, kVolume, &v) && v == 255 << 8, "track 1 driven there");
        d.scene_before = 5;  // the join's scene pair still guards it
        require(s.put(0, d) && !window_at(s[0], at(0, 1, 0, 64), &w), "another scene playing before the join: silent");
    }
    // Build 025: a song that doesn't loop plays a window at its end (pinned to the join into
    // entry 1, ending at or before it) in its last entry; one across the song's start plays
    // only from the start.
    {
        Variation end = {};
        end.song = 0, end.entry = 0, end.offset = -32, end.length = 32;
        end.scene_before = kNoScene, end.scene_after = kNoScene;
        SongPosition last = {};
        last.playing = true, last.song = 0, last.entry = 1, last.next_entry = 0;
        last.length = 64 * kStepTicks, last.position = 40 * kStepTicks;
        last.scene = kNoScene, last.scene_before = kNoScene;
        last.stops = true;
        u32 at;
        require(window_at(end, last, &at) && at == 8 * kStepTicks, "the song's end: plays in the last entry");
        Variation across = end;
        across.offset = -16;
        last.position = 56 * kStepTicks;
        require(!window_at(across, last, &at), "across the song's start: nothing before the song stops");
        last.stops = false;
        require(window_at(across, last, &at), "looping: across, as before");
    }
    // Build 047: a stutter lane's line is its density: on the notes, but it drives nothing (neither the
    // notes line, nor a word), in the window or in its eases.
    {
        Store s;
        Draft d = {};
        d.song = 0, d.entry = 1, d.offset = 0, d.length = 32;
        d.scene_before = d.scene_after = kNoScene;
        for (DraftLane& l : d.lanes) clear_lane(l);
        d.lanes[0].gesture = kGestureStutter, d.lanes[0].amount = 128, d.lanes[0].selection = 0x01, d.lanes[0].target = kTargetNotes;
        d.lanes[0].count = 2;
        d.lanes[0].points[0] = Point{0, 255, 0}, d.lanes[0].points[1] = Point{31, 255, 0};  // far from as set: would drive
        d.lanes[1] = d.lanes[0];
        d.lanes[1].target = kVolume;  // even on the volume
        require(s.put(0, d), "a stutter lane stores");
        Targets t;
        targets(s, at(0, 1, 8, 64), &t, 960);
        require(!t.notes_set[0] && t.count == 0, "a stutter lane drives nothing");
        targets(s, at(0, 1, 0, 64), &t, 960);
        require(!t.notes_set[0] && t.count == 0, "nor at its start (the ease before)");
    }
    // Build 047: a muted lane plays nothing (targets and their eases); the other lane still does.
    {
        Store s;
        Draft d = {};
        d.song = 0, d.entry = 1, d.offset = 0, d.length = 32;
        d.scene_before = d.scene_after = kNoScene;
        for (DraftLane& l : d.lanes) clear_lane(l);
        for (u32 n = 0; n < kLanesV10; ++n) {
            DraftLane& l = d.lanes[n];
            l.gesture = 0, l.amount = 128, l.selection = 1u << n, l.target = n ? kCutoff : kVolume;
            l.count = 2, l.points[0] = Point{0, 0, 0}, l.points[1] = Point{31, 255, 0};
        }
        require(s.put(0, d), "two lanes");
        Targets t;
        targets(s, at(0, 1, 8, 64), &t, 960);
        u32 v;
        require(t.find(0, kVolume, &v) && t.find(1, kCutoff, &v), "both play");
        d.lanes[0].muted = true;
        require(s.put(0, d) && s[0].lanes[0].muted, "lane a muted");
        targets(s, at(0, 1, 8, 64), &t, 960);
        require(!t.find(0, kVolume, &v) && t.find(1, kCutoff, &v) && t.count == 1, "lane a drives nothing, lane b still does");
        targets(s, at(0, 1, 0, 64), &t, 960);  // the ease before the window
        require(!t.find(0, kVolume, &v), "nor in its ease");
        targets(s, at(0, 1, 32, 64), &t, 960);  // the ease after
        require(!t.find(0, kVolume, &v), "nor after");
        d.lanes[1].muted = true;
        require(s.put(0, d), "both muted");
        targets(s, at(0, 1, 8, 64), &t, 960);
        require(t.count == 0, "both muted: nothing");
    }
    // Build 060 (version 11: "what about a third lane then"): three lanes play at once, each with its own
    // gesture, target and tracks, and each gives its own value (flat lines, so the values are exact): lane A a
    // custom line of 10 on track 1's volume, lane B a rise of 200 on tracks 2 and 3's cutoff, lane C a fall of 90
    // on tracks 4 and 5's send; every other lane leaves a track alone, and a muted lane C drops out alone.
    {
        Store s;
        Draft d = {};
        d.song = 0, d.entry = 1, d.offset = 0, d.length = 32;
        d.scene_before = d.scene_after = kNoScene;
        for (DraftLane& l : d.lanes) clear_lane(l);
        const u32 gesture[kLanes] = {0, 4, 5}, target[kLanes] = {kVolume, kCutoff, kSend}, selection[kLanes] = {0x01, 0x06, 0x18}, value[kLanes] = {10, 200, 90};
        for (u32 n = 0; n < kLanes; ++n) {
            DraftLane& l = d.lanes[n];
            l.gesture = gesture[n], l.amount = 128, l.selection = selection[n], l.target = target[n], l.default_target = false;
            l.count = 2, l.points[0] = Point{0, static_cast<unsigned char>(value[n]), 0}, l.points[1] = Point{31, static_cast<unsigned char>(value[n]), 0};
        }
        require(s.put(0, d) && s[0].lanes[2].gesture == 5 && s[0].lanes[2].target == kSend, "three lanes stored");
        Targets t;
        targets(s, at(0, 1, 8, 64), &t);
        u32 v;
        require(t.count == 5, "five words: one on track 1, two cutoffs, two sends");
        require(t.find(0, kVolume, &v) && v == 10 * 256, "lane A: track 1's volume");
        require(t.find(1, kCutoff, &v) && v == 200 * 256 && t.find(2, kCutoff, &v) && v == 200 * 256, "lane B: tracks 2 and 3's cutoff");
        require(t.find(3, kSend, &v) && v == 90 * 256 && t.find(4, kSend, &v) && v == 90 * 256, "lane C: tracks 4 and 5's send");
        require(!t.find(5, kSend, &v) && !t.find(0, kSend, &v) && !t.find(3, kCutoff, &v), "nothing on other tracks or words");
        d.lanes[2].muted = true;
        require(s.put(0, d), "lane C muted");
        targets(s, at(0, 1, 8, 64), &t);
        require(t.count == 3 && !t.find(3, kSend, &v) && t.find(1, kCutoff, &v), "lane C drops out alone");
        d.lanes[2].muted = false, d.lanes[0].muted = d.lanes[1].muted = true;
        require(s.put(0, d), "A and B muted");
        targets(s, at(0, 1, 8, 64), &t);
        require(t.count == 2 && t.find(3, kSend, &v) && v == 90 * 256, "lane C plays on its own");
        // The notes line from lane C (the notes target): the first lane to set a track's notes wins, as for A and B.
        d.lanes[0].muted = d.lanes[1].muted = false;
        d.lanes[2].gesture = 0, d.lanes[2].target = kTargetNotes, d.lanes[2].selection = 0x20;
        require(s.put(0, d), "lane C on the notes");
        targets(s, at(0, 1, 8, 64), &t);
        require(t.notes_set[5] && t.notes[5] == 90 * 256 && !t.notes_set[0], "lane C's notes line on track 6");
    }
    // Build 047 (version 8): a window that runs on across later joins. Pinned to the join into entry 2
    // (index 1), offset 16, 128 steps, over entries of 64 steps: it starts 16 steps in, crosses the join into
    // entry 3 (64 steps from the pin) and the one into entry 4 (128), and ends 16 steps into entry 4.
    {
        Store s;
        Draft d = {};
        d.song = 0, d.entry = 1, d.offset = 16, d.length = 128;
        d.scene_before = 0, d.scene_after = 1;
        d.reach = 2, d.reach_scene[0] = 2, d.reach_scene[1] = 3, d.reach_start[0] = 64, d.reach_start[1] = 128;
        for (DraftLane& l : d.lanes) clear_lane(l);
        d.lanes[0].gesture = 0, d.lanes[0].amount = 128, d.lanes[0].selection = 1, d.lanes[0].target = kVolume;
        d.lanes[0].count = 2, d.lanes[0].points[0] = Point{0, 0, 0}, d.lanes[0].points[1] = Point{127, 255, 0};
        require(s.put(0, d), "a long-form variation stores");
        auto scene_at = [](u32 entry, u32 step, u32 scene) {
            SongPosition p = at(0, entry, step, 64);
            p.scene = scene;
            return p;
        };
        u32 w;
        // Entry 1 (index 1): the window starts at step 16; entry 2 (index 2): at = (64 - 16 + step); entry 3: (128 - 16 + step).
                u32 sc, st;
        s.reach(s[0], 2, &sc, &st);
        require(sc == 3 && st == 128, "reach record 2");
        require(window_at(s[0], scene_at(1, 16, 1), &w, &s) && w == 0, "the join's own entry: starts at step 16");
        require(window_at(s[0], scene_at(1, 63, 1), &w, &s) && w == 47 * 480, "the join's entry: its last step");
        require(window_at(s[0], scene_at(2, 0, 2), &w, &s) && w == 48 * 480, "the next entry: runs on, no gap");
        require(window_at(s[0], scene_at(2, 63, 2), &w, &s) && w == 111 * 480, "the next entry's last step");
        require(window_at(s[0], scene_at(3, 0, 3), &w, &s) && w == 112 * 480, "the third entry: still running on");
        require(window_at(s[0], scene_at(3, 15, 3), &w, &s) && w == 127 * 480, "the window's last step, in entry 3");
        require(!window_at(s[0], scene_at(3, 16, 3), &w, &s), "past the window's end");
        require(!window_at(s[0], scene_at(4, 0, 4), &w, &s), "past the reach: nothing");
        // Continuity: one step on in the song is one step on in the window, across each join.
        u32 before, after;
        require(window_at(s[0], scene_at(1, 63, 1), &before, &s) && window_at(s[0], scene_at(2, 0, 2), &after, &s) && after - before == 480,
                "continuous across the first join");
        require(window_at(s[0], scene_at(2, 63, 2), &before, &s) && window_at(s[0], scene_at(3, 0, 3), &after, &s) && after - before == 480,
                "continuous across the second join");
        // A wrong scene in a later entry: silent. A cued scene that is the record's plays; none known trusts it.
        require(!window_at(s[0], scene_at(2, 10, 7), &w, &s), "a wrong scene in entry E+1: silent");
        require(window_at(s[0], scene_at(2, 10, kNoScene), &w, &s), "scene not known: the record is trusted");
        // Play or a jump into a middle entry plays from there (no arrival check).
        SongPosition jump = scene_at(2, 10, 2);
        jump.restarted = true, jump.scene_before = 5;
        require(window_at(s[0], jump, &w, &s) && w == 58 * 480, "a jump into the middle entry: plays from there");
        // Without the store, a reach is not looked at.
        require(!window_at(s[0], scene_at(2, 0, 2), &w), "no store: no reach");
        // ahead(): the next pass, over the boundary into entry E+1, finds the window (the scene not known there).
        SongPosition near_end = scene_at(1, 63, 1);
        near_end.position = 63 * 480 + 400;
        const SongPosition next = ahead(near_end, 480);
        require(next.entry == 2 && window_at(s[0], next, &w, &s) && w == 48 * 480 + 400, "ahead() across the boundary finds it");
        // Targets there follow the line across the join.
        Targets t;
        u32 v0, v1;
        targets(s, scene_at(1, 63, 1), &t, 0);
        require(t.find(0, kVolume, &v0), "driven in entry E");
        targets(s, scene_at(2, 0, 2), &t, 0);
        require(t.find(0, kVolume, &v1) && v1 > v0, "driven in entry E+1, the line rising on");
        targets(s, scene_at(3, 15, 3), &t, 0);
        require(t.find(0, kVolume, &v1) && v1 > 250 * 256, "driven to the end");
        targets(s, scene_at(3, 40, 3), &t, 0);
        require(!t.find(0, kVolume, &v1), "and nothing after");
    }
    // Build 058 (version 10): a window that runs back across earlier joins. Pinned to the join into entry 4, from
    // 200 steps before it for 250 steps (ending 50 steps after it), over entries of 64 steps: it crosses the join
    // into entry 3 (64 steps before the pin: the "section before", from the live length), entry 2 (128) and
    // entry 1 (192), not the one into entry 0 (256).
    {
        Store s;
        Draft d = {};
        d.song = 0, d.entry = 4, d.offset = -200, d.length = 250;
        d.scene_before = 3, d.scene_after = 4;
        d.back = 3, d.back_scene[0] = 3, d.back_scene[1] = 2, d.back_scene[2] = 1;
        d.back_start[0] = 64, d.back_start[1] = 128, d.back_start[2] = 192;
        for (DraftLane& l : d.lanes) clear_lane(l);
        d.lanes[0].gesture = 0, d.lanes[0].amount = 128, d.lanes[0].selection = 1, d.lanes[0].target = kVolume;
        d.lanes[0].count = 2, d.lanes[0].points[0] = Point{0, 0, 0}, d.lanes[0].points[1] = Point{249, 255, 0};
        require(s.put(0, d), "a variation reaching back stores");
        auto scene_at = [](u32 entry, u32 step, u32 scene) {
            SongPosition p = at(0, entry, step, 64);
            p.scene = scene;
            return p;
        };
        u32 w;
        // Entry 1 starts 192 steps before the join: 8 steps into the window. Entry 2: 72. Entry 3 (the section before): 136. Entry 4: 200.
        require(window_at(s[0], scene_at(1, 0, 1), &w, &s) && w == 8 * 480, "three entries back: the window's 8th step");
        require(window_at(s[0], scene_at(1, 63, 1), &w, &s) && w == 71 * 480, "three entries back, its last step");
        require(window_at(s[0], scene_at(2, 0, 2), &w, &s) && w == 72 * 480, "two entries back: runs on, no gap");
        require(window_at(s[0], scene_at(2, 63, 2), &w, &s) && w == 135 * 480, "two entries back, its last step");
        require(window_at(s[0], scene_at(3, 0, 3), &w, &s) && w == 136 * 480, "the section before: as it always was");
        require(window_at(s[0], scene_at(4, 0, 4), &w, &s) && w == 200 * 480, "the join's own entry");
        require(window_at(s[0], scene_at(4, 49, 4), &w, &s) && w == 249 * 480, "the window's last step");
        require(!window_at(s[0], scene_at(4, 50, 4), &w, &s), "past the window's end");
        require(!window_at(s[0], scene_at(0, 63, 0), &w, &s), "four entries back: outside its reach");
        require(!window_at(s[0], scene_at(5, 0, 5), &w, &s), "after its entry: nothing (no forward reach)");
        // The same song step by step: one step on in the song is one step on in the window across each join.
        u32 before, after;
        require(window_at(s[0], scene_at(1, 63, 1), &before, &s) && window_at(s[0], scene_at(2, 0, 2), &after, &s) && after - before == 480,
                "continuous across the join into entry 2");
        require(window_at(s[0], scene_at(2, 63, 2), &before, &s) && window_at(s[0], scene_at(3, 0, 3), &after, &s) && after - before == 480,
                "continuous across the join into entry 3");
        // The scene guard: a record whose scene no longer matches the scene playing keeps its entry silent; unknown trusts it.
        require(!window_at(s[0], scene_at(2, 10, 7), &w, &s), "a wrong scene two back: silent");
        require(!window_at(s[0], scene_at(1, 10, 2), &w, &s), "a wrong scene three back: silent");
        require(window_at(s[0], scene_at(2, 10, kNoScene), &w, &s) && w == 82 * 480, "scene not known: the record is trusted");
        // A record that does not know its scene plays whatever scene is playing.
        Draft loose = d;
        loose.back_scene[1] = kNoScene;
        require(s.put(0, loose) && window_at(s[0], scene_at(2, 10, 7), &w, &s), "an unknown record scene plays any scene");
        require(s.put(0, d), "restored");
        // Play or a jump into a middle entry plays from there (no arrival check).
        SongPosition jump = scene_at(2, 10, 2);
        jump.restarted = true, jump.scene_before = 5;
        require(window_at(s[0], jump, &w, &s) && w == 82 * 480, "a jump into the middle entry: plays from there");
        // Without the store the back list is not looked at.
        require(!window_at(s[0], scene_at(2, 0, 2), &w), "no store: no back reach");
        // ahead(): the next pass, over the boundary into entry 2, finds the window (the scene not known there).
        SongPosition near_end = scene_at(1, 63, 1);
        near_end.position = 63 * 480 + 400;
        const SongPosition next = ahead(near_end, 480);
        require(next.entry == 2 && window_at(s[0], next, &w, &s) && w == 72 * 480 + 400, "ahead() across the boundary finds it");
        // Targets follow the line from far back, and its lead (the ease) sits before entry 1.
        Targets t;
        u32 v0, v1;
        targets(s, scene_at(1, 0, 1), &t, 0);
        require(t.find(0, kVolume, &v0), "driven three entries back");
        targets(s, scene_at(2, 0, 2), &t, 0);
        require(t.find(0, kVolume, &v1) && v1 > v0, "driven two back, the line rising on");
        targets(s, scene_at(0, 63, 0), &t, 0);
        require(!t.find(0, kVolume, &v1), "not four back");
        // Build 064 ("from start" spanning the song): a window starting exactly on entry 1's start (192 steps back) plays
        // entry 1, from its first step (058 asserted the opposite: back 2, "a window starting on a join does not reach into
        // the entry before it", which left a song-long variation silent for its whole first scene).
        Draft shorter = d;
        shorter.offset = -192, shorter.back = 3, shorter.length = 242;
        shorter.lanes[0].points[1].step = 241;
        require(s.put(0, shorter), "a window starting on a join stores with that join recorded");
        require(window_at(s[0], scene_at(1, 0, 1), &w, &s) && w == 0, "build 064: a window starting on entry 1's start plays it, from the first step");
        require(window_at(s[0], scene_at(1, 63, 1), &before, &s) && window_at(s[0], scene_at(2, 0, 2), &after, &s) && after - before == 480 && after == 64 * 480,
                "build 064: continuous across the first join");
        require(!window_at(s[0], scene_at(0, 63, 0), &w, &s), "and not the entry before the window's start");
        // The old shape (back 2, entry 1 unrecorded) is still a valid record, and leaves entry 1 silent.
        Draft unrecorded = shorter;
        unrecorded.back = 2;
        require(s.put(0, unrecorded) && !window_at(s[0], scene_at(1, 63, 1), &w, &s), "an unrecorded join stays silent");
        // A start_j past the window's start does not store: the last record must be reached (offset + start <= 0).
        Draft past = shorter;
        past.offset = -191;
        require(!s.put(0, past), "a last record the window does not reach is refused");
        require(s.put(0, shorter), "restored");
        // Both lists at once: pinned at entry 3, from 150 steps before it for 300: back across the joins into
        // entries 2 (64) and 1 (128), forward across the one into entry 4 (64 after).
        Draft both = {};
        both.song = 0, both.entry = 3, both.offset = -150, both.length = 300;
        both.scene_before = 2, both.scene_after = 3;
        both.back = 2, both.back_scene[0] = 2, both.back_scene[1] = 1, both.back_start[0] = 64, both.back_start[1] = 128;
        both.reach = 1, both.reach_scene[0] = 4, both.reach_start[0] = 64;
        for (DraftLane& l : both.lanes) clear_lane(l);
        require(s.put(0, both), "a window with both lists");
        require(window_at(s[0], scene_at(1, 0, 1), &w, &s) && w == 22 * 480, "both lists: back to entry 1");
        require(window_at(s[0], scene_at(3, 0, 3), &w, &s) && w == 150 * 480, "both lists: its own entry");
        require(window_at(s[0], scene_at(4, 0, 4), &w, &s) && w == 214 * 480, "both lists: forward to entry 4");
        require(!window_at(s[0], scene_at(0, 0, 0), &w, &s) && !window_at(s[0], scene_at(5, 0, 5), &w, &s), "both lists: no further either way");
    }
    // Build 074: the climb out of a window has its own, longer length (ease_out), scaled by the jump; it
    // arrives exactly at as set; break and drop stay sudden; a window that follows closely wins the word.
    {
        const u32 ease = 6 * 20, ease_out = 32 * 20;  // ticks, at 20 a pass
        auto lane = [](Draft& d, u32 n, u32 gesture, u32 value) {
            DraftLane& l = d.lanes[n];
            l.gesture = gesture, l.amount = 128, l.selection = 1, l.target = kVolume, l.count = 2;
            l.points[0] = Point{0, static_cast<u8>(value), 0}, l.points[1] = Point{31, static_cast<u8>(value), 0};
        };
        auto draft = [&](u32 offset, u32 gesture, u32 value) {
            Draft d = {};
            d.song = 0, d.entry = 1, d.offset = static_cast<i32>(offset), d.length = 32;
            d.scene_before = d.scene_after = kNoScene;
            for (DraftLane& l : d.lanes) clear_lane(l);
            lane(d, 0, gesture, value);
            return d;
        };
        auto after = [&](u32 done) {  // `done` ticks into the climb out of a window that ends at step 32
            SongPosition p = at(0, 1, 32, 64);
            p.position = 32 * kStepTicks + done - 1;
            return p;
        };
        // out_length: the ease in at no jump, the whole ease out at the biggest, in between proportional.
        require(out_length(ease, ease_out, 0) == ease && out_length(ease, ease_out, kMiddle) == ease_out, "ends of the length");
        require(out_length(ease, ease_out, kMiddle / 2) == (ease + ease_out) / 2, "half the jump, half way");
        require(out_length(ease, 0, kMiddle) == ease && out_length(ease, ease, kMiddle) == ease, "no ease out: the ease in's length");
        // A fade to silence: climbs over ease_out ticks, not ease, rising all the way, exactly as set at the end.
        Store s;
        require(s.put(0, draft(0, 0, 0)), "fade to silence");
        Targets t;
        u32 v, last = 0;
        for (u32 done = 1; done <= ease_out; ++done) {
            targets(s, after(done), &t, ease, nullptr, ease_out);
            require(t.find(0, kVolume, &v), "still climbing");
            require(v >= last && (done == 1 || v > last), "rises every tick");
            require(done > ease || v < kMiddle, "after the old ease length it is still short of as set");
            last = v;
        }
        require(last == kMiddle, "arrives exactly at as set");
        targets(s, after(ease_out + 1), &t, ease, nullptr, ease_out);
        require(!t.find(0, kVolume, &v), "past it, given back (as set already)");
        // Without ease_out: the ease in's length, as before.
        targets(s, after(ease), &t, ease);
        require(t.find(0, kVolume, &v) && v == kMiddle, "default: as long as the ease in");
        targets(s, after(ease + 1), &t, ease);
        require(!t.find(0, kVolume, &v), "default: over after the ease in's length");
        // A small step (just over kEaseJump): about the ease in's length; a step under it: no ease.
        Store small;
        require(small.put(0, draft(0, 0, 128 - 33)), "a small step");
        const u32 small_len = out_length(ease, ease_out, 33 << 8);
        require(small_len > ease && small_len < ease_out / 2 + 1, "small step: a fraction of the full length (a quarter of the extra)");
        targets(small, after(small_len), &t, ease, nullptr, ease_out);
        require(t.find(0, kVolume, &v) && v == kMiddle, "small step: back at as set at its own length");
        targets(small, after(small_len + 1), &t, ease, nullptr, ease_out);
        require(!t.find(0, kVolume, &v), "small step: not dragged out to the full length");
        Store tiny;
        require(tiny.put(0, draft(0, 0, 128 - 10)), "a tiny step");
        targets(tiny, after(1), &t, ease, nullptr, ease_out);
        require(!t.find(0, kVolume, &v), "under kEaseJump: no ease");
        // Break and drop: no ease after, whatever ease_out is.
        for (u32 g = 2; g <= 3; ++g) {
            Store b;
            require(b.put(0, draft(0, g, 0)), "break or drop");
            targets(b, after(1), &t, ease, nullptr, ease_out);
            require(!t.find(0, kVolume, &v), "break and drop come back at once");
        }
        // A window that follows closely wins the word in the climb's tail, and the climb never drives
        // under a window; two variations close together: the first listed (store order) holds the word.
        Store two;
        require(two.put(0, draft(0, 0, 0)) && two.put(1, draft(32, 0, 255)), "two windows");
        targets(two, after(10), &t, ease, nullptr, ease_out);
        require(t.find(0, kVolume, &v) && v == 255 << 8 && t.count == 1, "the window that follows wins the word");
        Store gap;
        require(gap.put(0, draft(0, 0, 0)) && gap.put(1, draft(40, 0, 255)), "a window 8 steps on");
        targets(gap, after(10), &t, ease, nullptr, ease_out);  // 10 ticks after A, B's ease in has not begun
        require(t.find(0, kVolume, &v) && v < kMiddle && t.count == 1, "the climb out alone");
        targets(gap, after(7 * 480), &t, ease, nullptr, ease_out);  // inside B's lead (6 passes = 120 ticks)? no: 1 step before
        require(t.count <= 1, "one drive per word, never two");
        SongPosition inside = at(0, 1, 40, 64);
        targets(gap, inside, &t, ease, nullptr, ease_out);
        require(t.find(0, kVolume, &v) && v == 255 << 8 && t.count == 1, "inside the second window: its own value");
    }
    std::puts("variations window: ok");
    return 0;
}
