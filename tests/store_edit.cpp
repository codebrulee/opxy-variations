// Host test: editing the store (the page's put and remove), in place, within
// 8192 bytes and 64 variations, starting from vartest as build 003 saved it
// (version 1, rewritten as version 6 on load); the bytes after the edits below
// are what tools/variations.py writes for fixtures/edits.json
// (test_variations_tool.py decodes them).
#include "../gestures.hpp"
#include <cstdio>
#include <cstdlib>
using namespace variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
// examples/first-test.json, encoded as version 1: two custom variations at the join into entry 2
// (three as version 6).
static const u8 first[] = {86, 82, 1, 2, 0, 1, 224, 255, 32, 0, 0, 0, 3, 0, 0, 2, 16, 2, 0, 128, 0, 31, 0, 0, 33, 2, 0, 0, 0, 31, 255, 40, 0, 1, 32, 0, 32, 0, 0, 0, 12, 0, 0, 3, 50, 2, 0, 128, 0, 31, 0, 196, 3, 2, 0, 255, 0, 31, 0, 0, 19, 3, 0, 128, 0, 15, 255, 0, 31, 255, 0};
// fixtures/edits.json, encoded.
static const u8 expected[] = {86, 82, 12, 2, 0, 1, 240, 255, 16, 0, 0, 255, 255, 2, 200, 15, 3, 3, 0, 3, 0, 128, 0, 3, 128, 0, 4, 0, 127, 4, 128, 16, 12, 0, 60, 2, 0, 128, 0, 15, 192, 60, 255, 128, 255, 0, 2, 0, 0, 1, 0, 4, 0, 64, 0, 0, 255, 255, 5, 255, 16, 228, 2, 196, 2, 0, 128, 0, 63, 0, 196, 7, 100, 11, 1, 0, 216, 3, 0, 128, 0, 32, 78, 216, 63, 128, 40, 255, 128, 255, 0, 2, 0, 0};
static Store store;
static Draft d;
static u8 copy[kCapacity];
static void lane(u32 n, u32 gesture, u32 amount, u32 target, u32 selection, bool tracks, bool follows) {
    DraftLane& l = d.lanes[n];
    clear_lane(l);
    l.gesture = gesture, l.amount = amount, l.target = target, l.selection = selection;
    l.default_tracks = tracks, l.default_target = follows;
    l.curve = default_curve(gesture);
}
static void make(u32 song, u32 entry, i32 offset, u32 length) {
    d.song = song, d.entry = entry, d.offset = offset, d.length = length;
    d.unplaced = false, d.scene_before = d.scene_after = kNoScene;
    clear_lane(d.lanes[0]), clear_lane(d.lanes[1]), clear_lane(d.lanes[2]);
}
static bool same(const u8* a, u32 n) {
    if (store.size() != n) return false;
    for (u32 i = 0; i < n; ++i)
        if (store.bytes()[i] != a[i]) return false;
    return true;
}
int main() {
    require(store.load(first, sizeof first) && store.count() == 3 && store.version() == 12,
            "loads, as version 7 (the second's third line a variation of its own)");
    require(store[2].lanes[0].target == kCutoff && store[2].offset == 32 && store.remove(2) && store.count() == 2,
            "that one removed");
    const u32 generation = store.generation();
    // The first replaced by a break with a rise in lane b (a different size), a drop added,
    // the second removed, the drop replaced by a longer fall with a dip in lane b.
    make(0, 1, -16, 16);
    lane(0, brk, 200, kVolume, 0x03, true, true);
    lane(1, rise, 128, kCutoff, 0x0c, false, false);
    generate(d);
    require(store.put(0, d) && store.count() == 2, "replaces the first");
    require(store[0].lanes[0].gesture == brk && store[1].offset == 32 && store[1].lanes[0].gesture == custom,
            "the second as it was");
    make(1, 0, 4, 64);
    lane(0, drop, 128, kTargetNotes, 0xe4, false, false);
    generate(d);
    require(store.put(2, d) && store.count() == 3 && store[2].lanes[0].gesture == drop, "adds one");
    require(store.remove(1) && store.count() == 2 && store[1].lanes[0].gesture == drop, "removes one");
    make(1, 0, 4, 64);
    lane(0, fall, 255, kCutoff, 0xe4, false, true);
    lane(1, dip, 100, 11, 0x01, false, false);
    generate(d);
    require(store.put(1, d) && store[1].lanes[0].gesture == fall, "replaces the last");
    require(same(expected, sizeof expected), "the tool's bytes for fixtures/edits.json");
    require(store.generation() == generation, "edits keep the generation");
    // A draft read back is the same variation.
    store.draft(1, d);
    require(d.lanes[1].target == 11 && d.lanes[1].count == 3, "lane b in the draft");
    require(store.put(1, d) && same(expected, sizeof expected), "draft and put again: unchanged");
    // The bytes load as they are (saved and reopened).
    for (u32 i = 0; i < store.size(); ++i) copy[i] = store.bytes()[i];
    const u32 size = store.size();
    require(store.load(copy, size) && store.count() == 2 && store.generation() != generation, "reloads");
    // Refusals change nothing.
    make(0, 1, -16, 16);
    lane(0, brk, 200, kVolume, 0x03, true, true);
    generate(d);
    d.lanes[0].points[1].step = 0;  // out of order
    require(!store.put(0, d) && same(copy, size), "an invalid draft is refused");
    make(0, 1, -8192, 16);  // build 063: 8191 steps either way (-2047 until version 12)
    require(!store.put(0, d) && same(copy, size), "an offset past 512 bars before the join is refused");
    make(0, 1, -16, 16);
    lane(1, rise, 128, 12, 0x01, false, false);  // play mode: never a target
    require(!store.put(0, d) && same(copy, size), "a word that isn't a target is refused");
    lane(1, 19, 128, kCutoff, 0x01, false, false);
    require(!store.put(0, d) && same(copy, size), "an unknown gesture is refused");
    require(!store.put(5, d) && !store.remove(2), "no such variation");
    // Full by bytes: the largest variation (9 + three lines of 32 points, 318 bytes; build 060: 215 with two
    // lanes, 1591 in version 4) fits as many times as the bytes beside the others (the 3 already there) allow.
    make(0, 0, -128, 128);
    for (u32 n = 0; n < kLanes; ++n) {
        lane(n, custom, 0, n == 0 ? kVolume : n == 1 ? kCutoff : kSend, 0xff, false, false);
        d.lanes[n].count = kDraftPoints;
        for (u32 k = 0; k < kDraftPoints; ++k) d.lanes[n].points[k] = {static_cast<u8>(k * 4), static_cast<u8>(k), 0};
    }
    require(encoded_size(d) == 9 + kLanes * (kLaneHeader + 3 * kDraftPoints) && encoded_size(d) == 318, "the largest variation");
    const u32 room = (kCapacity - store.size()) / 318;
    u32 added = 0;
    while (store.count() < kMaxVariations && store.size() + 318 <= kCapacity) {
        require(store.put(store.count(), d), "large ones fit while there is room");
        ++added;
    }
    require(added == room && added > 20, "as many of them as the room holds");
    for (u32 i = 0; i < store.size(); ++i) copy[i] = store.bytes()[i];
    const u32 full = store.size();
    require(!store.put(store.count(), d) && store.size() == full, "another does not, and nothing changes");
    for (u32 i = 0; i < full; ++i) require(store.bytes()[i] == copy[i], "nothing changes");
    require(store.size() <= kCapacity, "within 8192 bytes");
    // Removing them all: nothing to save; then a first one again.
    while (store.count()) require(store.remove(0), "removes");
    require(store.size() == 0, "none left: saves nothing");
    make(3, 7, 0, 8);
    lane(0, swell, 128, kSend, 0x01, true, false);
    generate(d);
    require(store.put(0, d) && store.count() == 1 && store.size() == 4 + encoded_size(d), "a first one again");
    // Full by count: 64.
    while (store.count() < kMaxVariations) require(store.put(store.count(), d), "up to 64");
    require(!store.put(store.count(), d) && store.count() == kMaxVariations, "not 65");
    require(store.put(10, d), "but replacing one still works");
    std::puts("variations store editing: ok");
    return 0;
}
