// Host test: the song page's marks (store.hpp marked_joins, join_of): placed
// variations mark the join that holds them now, found by their scenes after
// the song is edited, the loop's join at both ends; unplaced ones and ones
// whose scenes left the song mark nothing; repin agrees with the marks.
#include "../store.hpp"
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
static void add(u32 song, u32 entry, u32 before, u32 after, bool unplaced = false, i32 offset = -8, bool muted = false) {
    d.song = song, d.entry = entry, d.offset = offset, d.length = 16;
    d.unplaced = unplaced, d.scene_before = before, d.scene_after = after;
    clear_lane(d.lanes[0]), clear_lane(d.lanes[1]), clear_lane(d.lanes[2]);
    d.lanes[0].gesture = 3, d.lanes[0].amount = 0, d.lanes[0].selection = 1, d.lanes[0].target = kVolume;
    d.lanes[0].muted = muted;
    require(store.put(store.count(), d), "adds");
}
// The joins marked for song `song`, as a bit set of the first 64.
static unsigned long long marks(u32 song, const u8* scenes, u32 entries) {
    unsigned long long bits = 0;
    u32 calls = 0;
    marked_joins(store, song, scenes, entries, [&](u32 j, bool) {
        require(j <= entries, "a join of the song");
        if (j < 64) bits |= 1ull << j;
        ++calls;
    });
    return bits;
}
static unsigned long long bit(u32 j) { return 1ull << j; }

int main() {
    // Song 1: scenes 1 2 3 4. Variations on the joins 2>3 (into entry 2) and 4>1 (the loop's, into entry 0).
    const u8 song[] = {1, 2, 3, 4};
    add(0, 2, 2, 3);
    add(0, 0, 4, 1);
    require(marks(0, song, 4) == (bit(2) | bit(0) | bit(4)), "the joins, the loop's at both ends");
    require(marks(1, song, 4) == 0, "only its song");
    // An entry inserted before: scenes 1 9 2 3 4. The store still says entry 2; the marks follow the scenes.
    const u8 inserted[] = {1, 9, 2, 3, 4};
    require(marks(0, inserted, 5) == (bit(3) | bit(0) | bit(5)), "moved with its scenes");
    // An entry deleted: scenes 1 3 4. Join 2>3 is gone: unmarked (the page will unplace it).
    const u8 deleted[] = {1, 3, 4};
    require(marks(0, deleted, 3) == (bit(0) | bit(3)), "a join that left the song marks nothing");
    // Two joins with its scenes: the nearer its old entry, the later of two as near.
    const u8 twice[] = {2, 3, 2, 3, 1, 4};
    store.clear();
    add(0, 2, 2, 3);
    require(marks(0, twice, 6) == bit(3), "the later of two as near");  // joins 1 and 3 are both 1 away
    // Unplaced variations are not marked, even when their scenes meet again.
    store.clear();
    add(0, 2, 2, 3, true);
    require(marks(0, song, 4) == 0, "unplaced: no mark");
    // Scenes not known (version 1 data): its entry, while the song reaches it.
    store.clear();
    add(0, 3, kNoScene, kNoScene);
    require(marks(0, song, 4) == bit(3), "scenes not known: where it is");
    require(marks(0, deleted, 3) == 0, "past the song's end: none");
    // repin agrees: after it the store's entries are the marked joins.
    store.clear();
    add(0, 2, 2, 3);
    add(0, 3, 3, 4);
    add(0, 1, 1, 2);  // 1 > 2 is gone once 9 is inserted between them
    const unsigned long long before = marks(0, inserted, 5);
    require(repin(store, 0, inserted, 5) == 3, "two move, one is unplaced");
    require(!store[0].unplaced && !store[1].unplaced && store[2].unplaced, "the gone one unplaced");
    require(before == (bit(store[0].entry) | bit(store[1].entry)) && before == (bit(3) | bit(4)), "marks are where repin puts them");
    require(marks(0, inserted, 5) == before, "and stay there after it");
    // The join into entry 0 of a long song: the start and the end (96).
    store.clear();
    add(0, 0, 4, 1);
    u8 longest[96];
    for (u32 j = 0; j < 96; ++j) longest[j] = static_cast<u8>(j % 4 + 1);
    u32 seen[2] = {0, 0}, n = 0;
    marked_joins(store, 0, longest, 96, [&](u32 j, bool) { if (n < 2) seen[n] = j; ++n; });
    require(n == 2 && seen[0] == 0 && seen[1] == 96, "the loop's join at 0 and 96");
    // 64 variations, every join of a 96-entry song: bounded, each a join of the song.
    store.clear();
    for (u32 i = 0; i < kMaxVariations; ++i) add(0, i + 1, longest[i], longest[i + 1]);
    n = 0;
    marked_joins(store, 0, longest, 96, [&](u32 j, bool) { require(j <= 96, "in the song"); ++n; });
    require(n == kMaxVariations, "one mark each");
    // Build 047: the loop's join marks only the end its window lies on.
    store.clear();
    add(0, 0, 4, 1, false, -16);
    require(marks(0, song, 4) == bit(4), "wholly at the song's end: the end only");
    store.clear();
    add(0, 0, 4, 1, false, 0);
    require(marks(0, song, 4) == bit(0), "at the song's start: the start only");
    // Build 049: muted. A variation whose every lane with a gesture is muted marks its join hollow;
    // a second one there that plays makes it filled, in either order.
    {
        store.clear();
        add(0, 2, 2, 3, false, -8, true);
        bool quiet = false;
        u32 calls = 0;
        marked_joins(store, 0, song, 4, [&](u32 j, bool q) { quiet = q && j == 2; ++calls; });
        require(calls == 1 && quiet, "a muted variation: its join reported muted");
        opxy::extensions::SongMarks m = {{0, 0, 0, 0}, {0, 0, 0, 0}};
        song_marks_of(store, 0, song, 4, m);
        require(m.marked(2) && m.is_muted(2), "drawn hollow");
        add(0, 2, 2, 3);  // plays, after the muted one
        m = {{0, 0, 0, 0}, {0, 0, 0, 0}};
        song_marks_of(store, 0, song, 4, m);
        require(m.marked(2) && !m.is_muted(2), "one that plays there: filled");
        store.clear();
        add(0, 2, 2, 3);
        add(0, 2, 2, 3, false, -8, true);  // muted, after the one that plays
        m = {{0, 0, 0, 0}, {0, 0, 0, 0}};
        song_marks_of(store, 0, song, 4, m);
        require(m.marked(2) && !m.is_muted(2), "filled whichever comes first");
        // Lane b empty, lane a muted: muted. Lane b playing: not.
        Draft both = d;
        both.lanes[0].muted = true;
        both.lanes[1].gesture = 3, both.lanes[1].amount = 0, both.lanes[1].selection = 2, both.lanes[1].target = kVolume, both.lanes[1].muted = false;
        store.clear();
        require(store.put(0, both), "two lanes");
        require(!muted(store[0]), "lane b plays: not muted");
    }
    std::puts("variations marks: ok");
    return 0;
}
