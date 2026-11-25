// Host test: the scenes either side of a join (store.hpp): version 1 data
// (scenes not known) read, pin, repin keeping variations on their joins after
// the song is edited (insert, delete, move entries; unchanged: nothing moves),
// unplaced variations kept but not played, and window.hpp's scene guard.
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
// fixtures/legacy.json as version 1 (as build 003 saved it; test_variations_tool.py keeps it current).
static const u8 old[] = {86, 82, 1, 2, 1, 2, 224, 255, 32, 0, 1, 200, 7, 0, 1, 2, 0, 2, 0, 0, 0, 31, 255, 216, 17, 1, 16, 128, 0, 0, 0, 16, 0, 16, 0, 0, 0, 128, 0, 0, 1, 55, 1, 15, 10, 0};
static Store store;
// A variation on song 0 at `entry`, scenes as given (kNoScene: not known).
static Draft d;
static void add(u32 entry, u32 before, u32 after, i32 offset = -16) {
    d.song = 0, d.entry = entry, d.offset = offset, d.length = 16;
    d.unplaced = false, d.scene_before = before, d.scene_after = after;
    clear_lane(d.lanes[0]), clear_lane(d.lanes[1]), clear_lane(d.lanes[2]);
    DraftLane& l = d.lanes[0];
    l.gesture = 3 /* drop */, l.amount = 0, l.selection = 1, l.target = kVolume;
    l.count = 1, l.points[0] = {0, 0, 0};
    require(store.put(store.count(), d), "adds");
}
static bool at(u32 i, u32 entry, bool unplaced = false) { return store[i].entry == entry && store[i].unplaced == unplaced; }

int main() {
    // Version 1: rewritten as version 7 on load, scenes not known.
    require(store.load(old, sizeof old) && store.count() == 2 && store.version() == 12, "version 1 loads");
    require(store[0].scene_before == kNoScene && store[0].scene_after == kNoScene && !store[0].unplaced, "no scenes");
    const u32 size = store.size();
    require(store.pin(1, 5, 3, 4, false), "pins");
    require(store[1].entry == 5 && store[1].scene_before == 3 && store[1].scene_after == 4, "pinned");
    require(store[0].entry == 2 && store[0].scene_before == kNoScene, "the other as it was");
    const u32 h = store[1].start;
    require(store.size() == size && store.bytes()[h + 1] == 5 && store.bytes()[h + 7] == 3 && store.bytes()[h + 8] == 4,
            "in the bytes, in place");
    {
        static u8 copy[kCapacity];
        for (u32 i = 0; i < size; ++i) copy[i] = store.bytes()[i];
        require(store.load(copy, size) && store[1].entry == 5 && store[1].scene_after == 4, "and reloads");
        require(store.pin(1, 5, 3, 4, true) && store[1].unplaced && store.bytes()[h + 6] == 2, "unplaced, in the flags");
    }
    require(!store.pin(2, 0, 0, 0, false) && !store.pin(0, 96, 0, 0, false) && !store.pin(0, 0, 99, 0, false), "pin's refusals");

    // repin. Song: A B A C (scenes 0 1 0 2). Joins: 0 C>A, 1 A>B, 2 B>A, 3 A>C.
    store.clear();
    add(1, 0, 1);              // 0: A>B
    add(3, 0, 2);              // 1: A>C
    add(2, kNoScene, kNoScene); // 2: not known (version 1 data)
    add(1, 0, 1, 4);           // 3: A>B, inside B
    d.song = 1;
    require(store.put(store.count(), d), "another song's");  // 4: song 1, entry 1
    const u8 song[] = {0, 1, 0, 2};
    require(repin(store, 0, song, 4) == 1, "unchanged: only the unknown one is filled");
    require(at(0, 1) && at(1, 3) && at(3, 1), "nothing moves");
    require(at(2, 2) && store[2].scene_before == 1 && store[2].scene_after == 0, "scenes filled where it is: B>A");
    require(repin(store, 0, song, 4) == 0, "and again: nothing");

    // Insert an entry (C) before them all: C A B A C.
    const u8 inserted[] = {2, 0, 1, 0, 2};
    require(repin(store, 0, inserted, 5) == 4, "four follow");
    require(at(0, 2) && at(1, 4) && at(2, 3) && at(3, 2), "each a join later, offsets kept");
    require(store[3].offset == 4 && store[4].entry == 1 && store[4].song == 1, "another song's untouched");
    // Delete the first two entries: B A C. Joins: 0 C>B, 1 B>A, 2 A>C. No A>B any more.
    const u8 deleted[] = {1, 0, 2};
    require(repin(store, 0, deleted, 3) == 4, "deleted");
    require(at(1, 2) && at(2, 1), "A>C and B>A follow back");
    require(at(0, 2, true) && at(3, 2, true), "A>B is gone: kept where it was, unplaced");
    // Unplaced: neither played nor at its join for the window.
    SongPosition p = {true, 0, 2, 0, 0, 16 * 480, 2, kNoScene};
    u32 into;
    require(!window_at(store[0], p, &into) && !window_at(store[3], p, &into), "unplaced: not played");
    require(window_at(store[1], SongPosition{true, 0, 1, 2, 8 * 480, 16 * 480, 0, kNoScene}, &into), "A>C plays before its join");
    // Move entries: A>B back, somewhere else: C C A B. Joins: 3 A>B.
    const u8 moved[] = {2, 2, 0, 1};
    require(repin(store, 0, moved, 4) >= 2 && at(0, 3) && at(3, 3), "A>B again: placed at its new join");
    require(at(1, 2, true), "A>C now gone: unplaced");
    // Two joins with the same scenes, as near: the later (an insert before more likely than a delete).
    store.clear();
    add(1, 0, 1);  // A B A B: A>B at 1 and 3
    add(3, 0, 1);
    const u8 abab[] = {2, 0, 1, 0, 1};  // inserted before: A>B at 2 and 4
    require(repin(store, 0, abab, 5) == 2 && at(0, 2) && at(1, 4), "each to its own join");
    const u8 empty_song[] = {0};
    require(repin(store, 0, empty_song, 0) == 2 && at(0, 2, true), "no entries: unplaced (the extension skips empty songs)");

    // The song's start is found by its first scene alone (build 014): the scene before it is only
    // the loop's. Song A B C, a variation on the start (C>A) and one on A>B.
    store.clear();
    add(0, 2, 0, 0);
    add(1, 0, 1, 0);
    const u8 abc[] = {0, 1, 2};
    require(repin(store, 0, abc, 3) == 0 && at(0, 0) && at(1, 1), "as made: nothing moves");
    const u8 abcd[] = {0, 1, 2, 3};  // an entry added at the song's end: the start is D>A now
    require(join_of(store[0], abcd, 4) == 0, "the start still found at the start");
    require(repin(store, 0, abcd, 4) == 1 && at(0, 0) && store[0].scene_before == 3 && store[0].scene_after == 0,
            "stays at the start, taking the new scene before");
    require(at(1, 1) && store[1].scene_before == 0, "the other untouched");
    {
        bool marked[5] = {};
        marked_joins(store, 0, abcd, 4, [&](u32 j, bool) { marked[j] = true; });
        require(marked[0] && !marked[4] && marked[1], "marked at the start (its window starts on it; build 047: not the end too) and at A>B");
    }
    const u8 dabc[] = {3, 0, 1, 2};  // an entry inserted before the start: D A B C
    require(repin(store, 0, dabc, 4) >= 1 && at(0, 1) && at(1, 2), "a new first scene: it follows its scenes (D>A) to entry 2");

    // The scene guard: a variation with known scenes plays only where they are.
    store.clear();
    add(1, 0, 1, -8);  // straddles the join into entry 1: A>B
    SongPosition before = {true, 0, 0, 1, 12 * 480, 16 * 480, 0, kNoScene}, after = {true, 0, 1, 2, 2 * 480, 16 * 480, 1, kNoScene};
    require(window_at(store[0], before, &into) && window_at(store[0], after, &into), "plays either side");
    after.scene_before = 0;
    require(window_at(store[0], after, &into), "came from its scene before: plays");
    after.scene_before = 2;  // an entry inserted before it, its own scene the same: came from another scene
    require(!window_at(store[0], after, &into), "came from another scene: silent");
    after.scene_before = kNoScene;
    before.scene = 2, after.scene = 2;  // the song edited (or a cued scene) not yet re-pinned
    require(!window_at(store[0], before, &into) && !window_at(store[0], after, &into), "the wrong scene: silent");
    store.clear();
    add(1, kNoScene, kNoScene, -8);
    require(window_at(store[0], before, &into) && window_at(store[0], after, &into), "scenes not known: plays as before");
    // Build 047: a long-form window's reach follows the song's sections (reach_of, reach_repin).
    {
        const u8 scenes[5] = {0, 1, 2, 3, 4};
        const u32 steps[5] = {32, 32, 64, 16, 16};
        u8 scene[kMaxReach];  // build 063: reach_of's records are u8 and u16 as Draft's are
        u16 start[kMaxReach];
        // From the join into entry 1 (offset 16, 96 steps): ends at step 112 from the join: crosses the joins at 32 (into
        // entry 2) and 96 (into entry 3), not the one at 112.
        u32 n = reach_of(scenes, steps, 5, 1, 16, 96, scene, start);
        require(n == 2 && scene[0] == 2 && start[0] == 32 && scene[1] == 3 && start[1] == 96, "reach_of: two joins crossed");
        require(reach_of(scenes, steps, 5, 1, 0, 32, scene, start) == 0, "a window ending on the section's end crosses nothing");
        require(reach_of(scenes, steps, 5, 1, 0, 33, scene, start) == 1, "one step past: the next join");
        require(reach_of(scenes, steps, 5, 3, 0, 256, scene, start) == 1 && start[0] == 16 && scene[0] == 4,
                "never past the song's last entry (no wrap)");
        require(reach_of(scenes, steps, 5, 4, 0, 256, scene, start) == 0, "the last entry: none");
        require(reach_of(scenes, steps, 5, 1, -32, 32, scene, start) == 0, "a window before the join: none");
        // Ninety-five at most (thirty-one until version 12, fifteen until version 9): build 063, a 96-entry song of 8-step
        // scenes under a window of 4096 steps; a window that stops short stops at its end.
        u8 many[96];
        u32 lens[96];
        for (u32 i = 0; i < 96; ++i) many[i] = static_cast<u8>(i), lens[i] = 8;
        require(reach_of(many, lens, 96, 0, 0, 4096, scene, start) == kMaxReach && kMaxReach == 95 && start[kMaxReach - 1] == 8 * 95 && scene[94] == 95,
                "at most 95");
        require(reach_of(many, lens, 96, 0, 0, 256, scene, start) == 31 && start[30] == 8 * 31, "a 256-step window: the 31 joins it holds");
        require(reach_of(many, lens, 96, 1, 0, 4096, scene, start) == 94, "never past entry 95: 94 from entry 1");
        // A variation pinned to entry 1, 100 steps from step 0: reach from the song as it was.
        store.clear();
        Draft long_draft = {};
        long_draft.song = 0, long_draft.entry = 1, long_draft.offset = 0, long_draft.length = 100;
        long_draft.scene_before = 0, long_draft.scene_after = 1;
        for (DraftLane& l : long_draft.lanes) clear_lane(l);
        long_draft.reach = reach_of(scenes, steps, 5, 1, 0, 100, long_draft.reach_scene, long_draft.reach_start);
        require(long_draft.reach == 2 && store.put(0, long_draft) && store[0].reach == 2, "stored with its reach");
        add(0, kNoScene, kNoScene, 0);  // a short one on entry 0, which needs nothing
        require(reach_repin(store, 0, scenes, steps, 5) == 0 && store[0].reach == 2, "the song unchanged: nothing changes");
        // Entry 2 grows to 128 steps: the window (ends at 100) now ends inside entry 2: one join.
        const u32 grown[5] = {32, 32, 128, 16, 16};
        require(reach_repin(store, 0, scenes, grown, 5) == 1 && store[0].reach == 1 && store[0].length == 100, "a section grew: the reach shrinks, the length kept");
        // The song shortened to three entries: entry 3 is gone, the window plays where entries exist.
        const u32 shorter[5] = {32, 32, 16, 0, 0};
        require(reach_repin(store, 0, scenes, shorter, 3) == 0 && store[0].reach == 1 && store[0].length == 100, "entry 3 gone: the last join still crossed, nothing to change");
        const u32 two[5] = {32, 32, 0, 0, 0};
        require(reach_repin(store, 0, scenes, two, 2) == 1 && store[0].reach == 0 && store[0].length == 100, "no later entry: no reach");
        // And back to a longer song: a window that passes its entry's end gets its reach again.
        require(reach_repin(store, 0, scenes, steps, 5) == 1 && store[0].reach == 2, "grows again");
    }
    // Build 058 (version 10): the backward list follows the song's sections (back_reach_of, reach_repin).
    {
        const u8 scenes[6] = {0, 1, 2, kNoScene, 4, 5};
        const u32 steps[6] = {32, 32, 64, 16, 16, 16};
        u8 scene[kMaxReach];  // build 063: reach_of's records are u8 and u16 as Draft's are
        u16 start[kMaxReach];
        // Pinned to the join into entry 4, from 100 steps before it: entry 3 starts 16 before (crossed), entry 2 80 before
        // (crossed), entry 1 144 before (not): two joins crossed, scenes of entries 3 and 2.
        u32 n = back_reach_of(scenes, steps, 6, 4, -100, 100, scene, start);
        require(n == 2 && scene[0] == kNoScene && start[0] == 16 && scene[1] == 2 && start[1] == 80, "back_reach_of: two joins crossed, an unknown scene");
        // Build 064: a window starting exactly on an entry's start is recorded (the entry holding the start is reached; 058
        // recorded only a start strictly before it). The section before is read live, so its record is not played from.
        require(back_reach_of(scenes, steps, 6, 4, -16, 50, scene, start) == 1 && start[0] == 16, "build 064: a window starting on the join into the section before: that entry (inclusive)");
        require(back_reach_of(scenes, steps, 6, 4, -15, 50, scene, start) == 0, "one step short of it: nothing");
        require(back_reach_of(scenes, steps, 6, 4, -17, 50, scene, start) == 1 && start[0] == 16, "one step further back: that join");
        require(back_reach_of(scenes, steps, 6, 4, -80, 50, scene, start) == 2 && start[1] == 80, "build 064: starting exactly on the start of the entry two back: both recorded");
        require(back_reach_of(scenes, steps, 6, 4, 0, 50, scene, start) == 0 && back_reach_of(scenes, steps, 6, 4, 20, 50, scene, start) == 0,
                "a window at or after its join crosses nothing back");
        require(back_reach_of(scenes, steps, 6, 4, -2047, 2047, scene, start) == 4 && start[3] == 144 && scene[3] == 0,
                "never before entry 0 (no wrap): four joins at most from entry 4");
        require(back_reach_of(scenes, steps, 6, 0, -2047, 2047, scene, start) == 0, "entry 0: none, even in a looping song");
        // Ninety-five at most (31 until version 12; build 063: a 96-entry song of 8-step scenes, from the last entry).
        u8 many[96];
        u32 lens[96];
        for (u32 i = 0; i < 96; ++i) many[i] = static_cast<u8>(i), lens[i] = 8;
        require(back_reach_of(many, lens, 96, 95, -4095, 100, scene, start) == kMaxReach && start[kMaxReach - 1] == 8 * 95 && scene[0] == 94 && scene[94] == 0,
                "at most 95");
        require(back_reach_of(many, lens, 96, 95, -2047, 100, scene, start) == 95 && back_reach_of(many, lens, 96, 95, -255, 100, scene, start) == 31,
                "a window starting 255 steps back crosses 31 joins");
        // A section of 3000 steps: crossed (3000 < 4095), the next not (6000).
        u32 wide[96];
        for (u32 i = 0; i < 96; ++i) wide[i] = 3000;
        require(back_reach_of(many, wide, 96, 95, -4095, 100, scene, start) == 1 && start[0] == 3000, "long sections: the second join is past the window");
        // reach_repin: a variation pinned to entry 4, 100 steps back and 100 long; nothing changes while the song does not.
        store.clear();
        Draft d = {};
        d.song = 0, d.entry = 4, d.offset = -100, d.length = 100;
        d.scene_before = 3, d.scene_after = 4;
        for (DraftLane& l : d.lanes) clear_lane(l);
        d.back = back_reach_of(scenes, steps, 6, 4, -100, 100, d.back_scene, d.back_start);
        require(d.back == 2 && store.put(0, d) && store[0].back == 2, "stored with its back list");
        require(reach_repin(store, 0, scenes, steps, 6) == 0 && store[0].back == 2, "the song unchanged: nothing changes");
        // Entry 3 grows to 100 steps: the window now starts exactly on its start (build 064: that one record stays, inclusive;
        // 058: none), so only the section before is held and entry 2 is gone.
        const u32 grown[6] = {32, 32, 64, 100, 16, 16};
        require(reach_repin(store, 0, scenes, grown, 6) == 1 && store[0].back == 1 && store[0].offset == -100 && store[0].length == 100,
                "a section grew: the back list shrinks (064: to the one entry the start is on), the window kept");
        // Entry 3 shrinks to 4 steps: joins at 4 and 68 steps back are crossed, and (build 064, inclusive) the entry
        // starting exactly 100 back is recorded too; 058 stopped short of it (100 < 100).
        const u32 shrunk[6] = {32, 32, 64, 4, 16, 16};
        require(reach_repin(store, 0, scenes, shrunk, 6) == 1 && store[0].back == 3 && store[0].length == 100, "a section shrank: the list grows (064: three, the third on the window's start)");
        u32 sc = 0, st = 0;
        store.back(store[0], 2, &sc, &st);
        require(sc == 2 && st == 68, "back record 2 from the shrunk song");
        store.back(store[0], 3, &sc, &st);
        require(st == 100, "build 064: back record 3, on the window's start");
        // A song that stops sooner: entry 4 is gone, the variation has no join and is left alone.
        require(reach_repin(store, 0, scenes, steps, 3) == 0 && store[0].back == 3, "the pinned entry gone: left alone");
        // Forward and backward lists change together in one put: a window across both.
        store.clear();
        Draft both = {};
        both.song = 0, both.entry = 2, both.offset = -20, both.length = 100;
        both.scene_before = 1, both.scene_after = 2;
        for (DraftLane& l : both.lanes) clear_lane(l);
        require(store.put(0, both) && store[0].reach == 0 && store[0].back == 0, "stored with neither");
        require(reach_repin(store, 0, scenes, steps, 6) == 1 && store[0].reach == 1 && store[0].back == 0, "reach only: 80 steps past the 64 of entry 2");
        const u32 small[6] = {8, 8, 64, 16, 16, 16};
        require(reach_repin(store, 0, scenes, small, 6) == 1 && store[0].reach == 1 && store[0].back == 2, "back list gained alongside the forward one");
        require(reach_repin(store, 0, scenes, small, 6) == 0, "then nothing changes");
    }
    std::puts("variations repin: ok");
    return 0;
}
