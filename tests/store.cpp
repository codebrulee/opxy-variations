// Host test: store.hpp reads what tools/variations.py writes (format version
// 5: a window and two lanes, each with its curve and one line), refuses
// damage, and rewrites versions 1-4 as version 6 as they load (version 5 renumbered), as the tool's
// upgrade() does (build and break made again, so the stagger reproduces
// version 4's per-track entries and exits).
#include "../window.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static Store store;
// fixtures/build.json as tools/variations.py encodes it (fixtures/build.bin);
// test_variations_tool.py checks these arrays are still the tool's.
static u8 data[] = {86, 82, 12, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 2, 200, 15, 7, 3, 0, 2, 0, 128, 0, 31, 0, 216, 4, 128, 11, 16, 32, 60, 2, 0, 128, 0, 31, 192, 60, 255, 128, 255, 0, 2, 0, 0, 0, 0, 16, 0, 16, 0, 0, 2, 255, 0, 0, 64, 128, 0, 0, 1, 15, 10, 0, 255, 128, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0};
// build.json as version 4 had it (lane a's break with points on tracks 1 and
// 2; builds 015-023 saved it so), the same as version 3 (builds 013 and 014),
// and both as version 6: the break made again from its gesture, lane b track
// 5's line, the custom lane track 8's.
static const u8 v4[] = {86, 82, 4, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 2, 200, 15, 7, 3, 0, 2, 0, 2, 0, 128, 0, 31, 0, 216, 1, 1, 16, 128, 0, 4, 128, 11, 16, 0, 60, 1, 4, 2, 0, 128, 0, 31, 192, 60, 0, 0, 16, 0, 16, 0, 0, 2, 255, 0, 0, 64, 128, 0, 0, 1, 7, 1, 15, 10, 0, 255, 128, 255, 0, 2, 0, 0};
static const u8 v6[] = {86, 82, 12, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 2, 200, 15, 7, 3, 0, 3, 0, 128, 0, 7, 128, 0, 8, 0, 127, 4, 128, 11, 16, 0, 60, 2, 0, 128, 0, 31, 192, 60, 255, 128, 255, 0, 2, 0, 0, 0, 0, 16, 0, 16, 0, 0, 2, 255, 0, 0, 64, 128, 0, 0, 1, 15, 10, 0, 255, 128, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0};
// build.json as version 6 saved it (no rates): loads as version 8 with every rate 1.
static const u8 was6[] = {86, 82, 6, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 2, 200, 15, 7, 3, 0, 2, 0, 128, 0, 31, 0, 216, 4, 128, 11, 16, 0, 60, 2, 0, 128, 0, 31, 192, 60, 0, 0, 16, 0, 16, 0, 0, 2, 255, 0, 0, 64, 128, 0, 0, 1, 15, 10, 0, 255, 128, 255, 0, 2, 0, 0};
static const u8 v3[] = {86, 82, 3, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 2, 200, 15, 7, 3, 2, 0, 2, 0, 128, 0, 31, 0, 216, 1, 1, 16, 128, 0, 4, 128, 11, 16, 0, 1, 4, 2, 0, 128, 0, 31, 192, 60, 0, 0, 16, 0, 16, 0, 0, 2, 255, 0, 0, 64, 128, 0, 1, 7, 1, 15, 10, 0, 255, 128, 255, 0, 2, 0};
// fixtures/legacy.json as versions 2 and 1 (builds 004-012 and 001-003 saved
// it so), and upgrade() of the version 2 bytes.
static const u8 legacy2[] = {86, 82, 2, 2, 1, 2, 224, 255, 32, 0, 1, 200, 7, 0, 1, 255, 255, 2, 0, 2, 0, 0, 0, 31, 255, 216, 17, 1, 16, 128, 0, 0, 0, 16, 0, 16, 0, 0, 0, 128, 0, 0, 255, 255, 1, 55, 1, 15, 10, 0};
static const u8 legacy1[] = {86, 82, 1, 2, 1, 2, 224, 255, 32, 0, 1, 200, 7, 0, 1, 2, 0, 2, 0, 0, 0, 31, 255, 216, 17, 1, 16, 128, 0, 0, 0, 16, 0, 16, 0, 0, 0, 128, 0, 0, 1, 55, 1, 15, 10, 0};
static const u8 upgraded[] = {86, 82, 12, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 1, 200, 15, 7, 3, 0, 3, 0, 0, 0, 7, 0, 0, 8, 128, 127, 0, 0, 64, 2, 0, 0, 1, 16, 128, 0, 255, 128, 255, 0, 2, 0, 0, 0, 0, 16, 0, 16, 0, 0, 255, 255, 0, 0, 16, 128, 0, 0, 1, 15, 10, 0, 255, 0, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0};
// Build 060: fixtures/legacy-four.json (every line a variation has: level, notes, send, filter in the first, three in the
// second) as versions 2 and 1, and the tool's upgrade of both: two variations of two lines each, then two more (lane C
// empty in all), and build.json as version 10 saved it (two lanes a variation).
static const u8 legacy4v2[] = {86, 82, 2, 2, 1, 2, 224, 255, 32, 0, 0, 0, 15, 0, 0, 255, 255, 4, 0, 2, 0, 0, 0, 31, 255, 0, 17, 1, 16, 128, 0, 34, 1, 4, 90, 0, 51, 2, 8, 10, 0, 20, 220, 0, 0, 0, 16, 0, 16, 0, 0, 0, 112, 0, 0, 255, 255, 3, 4, 1, 0, 255, 0, 37, 1, 7, 200, 0, 54, 1, 15, 30, 0};
static const u8 legacy4v1[] = {86, 82, 1, 2, 1, 2, 224, 255, 32, 0, 0, 0, 15, 0, 0, 4, 0, 2, 0, 0, 0, 31, 255, 0, 17, 1, 16, 128, 0, 34, 1, 4, 90, 0, 51, 2, 8, 10, 0, 20, 220, 0, 0, 0, 16, 0, 16, 0, 0, 0, 112, 0, 0, 3, 4, 1, 0, 255, 0, 37, 1, 7, 200, 0, 54, 1, 15, 30, 0};
static const u8 upgraded4[] = {86, 82, 12, 4, 1, 2, 224, 255, 32, 0, 0, 255, 255, 0, 0, 15, 1, 0, 0, 2, 0, 0, 0, 31, 128, 0, 0, 0, 64, 2, 0, 0, 1, 16, 128, 0, 255, 128, 255, 0, 2, 0, 0, 1, 2, 224, 255, 32, 0, 0, 255, 255, 0, 0, 22, 4, 0, 0, 1, 4, 173, 0, 0, 0, 16, 8, 0, 0, 2, 8, 10, 0, 20, 220, 0, 255, 128, 255, 0, 2, 0, 0, 0, 0, 16, 0, 16, 0, 0, 255, 255, 0, 0, 15, 16, 0, 0, 1, 0, 128, 0, 0, 0, 22, 32, 0, 0, 1, 7, 228, 0, 255, 128, 255, 0, 2, 0, 0, 0, 0, 16, 0, 16, 0, 0, 255, 255, 0, 0, 16, 64, 0, 0, 1, 15, 30, 0, 255, 0, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0};
static const u8 data10[] = {86, 82, 10, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 2, 200, 15, 7, 3, 0, 2, 0, 128, 0, 31, 0, 216, 4, 128, 11, 16, 32, 60, 2, 0, 128, 0, 31, 192, 60, 0, 0, 16, 0, 16, 0, 0, 2, 255, 0, 0, 64, 128, 0, 0, 1, 15, 10, 0, 255, 128, 255, 0, 2, 0, 0};
// Build 063: the same bytes as version 11 (build 060) saved them: within the smaller ranges, they load as version 12 and save back as its bytes.
static const u8 data11[] = {86, 82, 11, 2, 1, 2, 224, 255, 32, 0, 0, 255, 255, 2, 200, 15, 7, 3, 0, 2, 0, 128, 0, 31, 0, 216, 4, 128, 11, 16, 32, 60, 2, 0, 128, 0, 31, 192, 60, 255, 128, 255, 0, 2, 0, 0, 0, 0, 16, 0, 16, 0, 0, 2, 255, 0, 0, 64, 128, 0, 0, 1, 15, 10, 0, 255, 128, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0};
static bool same(const u8* a, u32 n) {
    if (store.size() != n) return false;
    for (u32 i = 0; i < n; ++i)
        if (store.bytes()[i] != a[i]) return false;
    return true;
}
// Build 060: version 11 bytes as version `version` (10 or below) held them, two lanes a variation: lane C, which must
// be empty, taken out of each variation (its last seven bytes) and the number changed. Returns the new size.
static u32 as_old(const u8* v11, u32 size, u32 version, u8* out) {
    static Store scratch;
    require(scratch.load(v11, size), "as_old: valid bytes");
    u32 at = 4;
    for (u32 i = 0; i < 4; ++i) out[i] = v11[i];
    for (u32 i = 0; i < scratch.count(); ++i) {
        const Variation& v = scratch[i];
        require(v.lanes[2].gesture == kNoGesture && v.lanes[2].count == 0, "as_old: lane C is empty");
        for (u32 k = v.start; k < v.end - kLaneHeader; ++k) out[at++] = v11[k];
    }
    out[2] = static_cast<u8>(version);
    return at;
}
int main() {
    const u32 n = sizeof data;
    require(store.load(data, n) && store.count() == 2 && store.version() == 12, "loads the tool's bytes (version 6)");
    const Variation& a = store[0];
    require(a.song == 1 && a.entry == 2 && a.offset == -32 && a.length == 32 && !a.unplaced, "the window");
    require(a.scene_before == kNoScene && a.scene_after == kNoScene, "scenes not known");
    const Lane& la = a.lanes[0];
    require(la.gesture == 2 && la.amount == 200 && la.target == 15 && la.selection == 0b111, "lane a");
    require(la.default_tracks && la.default_target, "lane a follows its gesture");
    const Lane& lb = a.lanes[1];
    require(lb.gesture == 4 && lb.target == 11 && lb.selection == 0x10 && !lb.default_tracks && !lb.default_target,
            "lane b: a rise on the release, its own tracks");
    u32 count;
    const Point* p = store.points(a, 0, &count);
    require(p && count == 2 && p[0].value == 128 && p[1].value == 0 && p[1].curve == -40, "lane a's line");
    p = store.points(a, 1, &count);
    require(p && count == 2 && p[1].value == 192, "lane b's line");
    const Variation& b = store[1];
    require(b.offset == 16 && b.scene_before == 2 && b.scene_after == kNoScene, "the second variation");
    require(b.lanes[0].target == kTargetNotes && b.lanes[1].gesture == kNoGesture && b.lanes[1].target == kNoTarget,
            "a notes lane, then an empty one");
    p = store.points(b, 0, &count);
    require(p && count == 1 && p[0].value == 10, "the notes line");
    require(!store.points(b, 1, &count) && count == 0, "the empty lane: no line");
    require(same(data, n), "saved back as loaded");
    // Build 041 (version 7): lane b's rate 4 (flags bits 4-5), lane a's 1; round-tripped through a draft.
    require(la.rate == 1 && lb.rate == 4 && b.lanes[0].rate == 1 && b.lanes[1].rate == 1, "the lanes' rates");
    {
        Draft d;
        store.draft(0, d);
        require(d.lanes[0].rate == 1 && d.lanes[1].rate == 4, "rate in the draft");
        d.lanes[0].rate = 8;
        require(store.put(0, d) && store[0].lanes[0].rate == 8 && store[0].lanes[1].rate == 4 && store.bytes()[4 + 9 + 4] >> 4 == 3,
                "put: rate 8 in flags bits 4-5");
        d.lanes[0].rate = 3;
        require(!store.put(0, d), "a rate of 3 is refused");
        require(store.load(data, n) && same(data, n), "as it was");
    }
    // Version 6 (no rates): loads as version 8 with rate 1, flags bits 4-5 then meant nothing.
    {
        require(store.load(was6, sizeof was6) && store.version() == 12 && store.count() == 2 && store.size() == sizeof was6 + 2 * kLaneHeader &&
                    store.bytes()[2] == 12 && store[0].lanes[1].rate == 1 && store[0].lanes[0].rate == 1,
                "version 6: rate 1, the number 11, an empty lane C written in each (build 060)");
        u8 six[sizeof was6];
        std::memcpy(six, was6, sizeof was6);
        six[4 + 9 + 4] |= 0x30;  // lane a's flags: bits 4-5 set
        require(store.load(six, sizeof six) && store[0].lanes[0].rate == 1 && store.bytes()[4 + 9 + 4] == was6[4 + 9 + 4],
                "version 6's flags bits 4-5 are ignored and cleared");
        require(store.load(data, n), "back to version 7");
    }
    // Damage anywhere: refused, holding none.
    for (u32 cut = 0; cut < n; ++cut) {
        require(!store.load(data, cut) || cut == 0, "a truncated copy is refused");
        require(store.count() == 0, "and holds none");
    }
    u8 bad[sizeof data];
    // 2 version (0xff: none known), 3 count, 4 song, 5 entry, 9 length's high byte (low 0xff is 255: build 047), 19 lane
    // a's point count, 20 a point's step (0xff is none for a gesture or target).
    const u32 flips[] = {0, 2, 3, 4, 5, 9, 19, 20};
    for (u32 at : flips) {
        std::memcpy(bad, data, n);
        bad[at] = 0xff;
        require(!store.load(bad, n), "a damaged field is refused");
    }
    std::memcpy(bad, data, n);
    bad[19] = 33;  // a line holds 32 points at most
    require(!store.load(bad, n), "too many points refused");
    std::memcpy(bad, data, n);
    bad[18] = 0x80;  // lane a's curve: -128 is none of -127..127
    require(!store.load(bad, n), "a curve out of range is refused");
    std::memcpy(bad, data, n);
    bad[13] = 19;  // gestures are 0-18 (8, 9 the fades: build 016; 10-13 build 040; 14 build 046; 15-17 build 048; 18 hold, build 062), or none
    require(!store.load(bad, n), "an unknown gesture is refused");
    std::memcpy(bad, data, n);
    bad[15] = 41;  // the Mix level: not a target
    require(!store.load(bad, n), "a word the SDK refuses is no target");
    std::memcpy(bad, data, n);
    bad[11] = 99;  // scenes are 0-98 or 0xff
    require(!store.load(bad, n), "a scene out of range is refused");
    std::memcpy(bad, data, n);
    bad[6] = 0x01, bad[7] = 0xe0;  // offset -8191: (version 5) wholly in the section before, as far as 512 bars (128 until build 063)
    require(store.load(bad, n) && store[0].offset == -8191, "a window far before the join");
    bad[6] = 0x00;  // -8192
    require(!store.load(bad, n), "past 512 bars before refused");
    // Build 063: version 11 bytes keep version 11's range, -2047, and are read as they were.
    std::memcpy(bad, data, n);
    bad[2] = 11, bad[6] = 0x01, bad[7] = 0xf8;  // -2047
    require(store.load(bad, n) && store[0].offset == -2047 && store.bytes()[2] == 12, "version 11: -2047 still loads");
    bad[6] = 0x00;  // -2048
    require(!store.load(bad, n), "version 11: past 128 bars before refused, as it was");
    require(store.load(data, 0) && store.count() == 0 && store.size() == 0, "no bytes: none");

    // Build 047 (version 8): a muted lane, a window up to 256 steps and a reach list; version 7 bytes
    // (as build 046 saved them) load as version 8 with neither.
    {
        require(store.load(data, n), "back to the tool's bytes");
        Draft d;
        store.draft(0, d);
        require(!d.lanes[0].muted && !d.lanes[1].muted && d.reach == 0, "a plain variation: no mute, no reach");
        d.lanes[1].muted = true;
        require(store.put(0, d) && store[0].lanes[1].muted && !store[0].lanes[0].muted, "put: a muted lane");
        Draft e;
        store.draft(0, e);
        require(e.lanes[1].muted && !e.lanes[0].muted, "muted in the draft");
        // Length 256, a reach of two joins.
        store.load(data, n);
        store.draft(0, d);
        d.length = 256, d.offset = 100;
        d.reach = 2, d.reach_scene[0] = 4, d.reach_scene[1] = kNoScene, d.reach_start[0] = 100, d.reach_start[1] = 300;
        require(store.put(0, d) && store[0].length == 256 && store[0].reach == 2, "put: a long window with a reach");
        u32 scene = 0, start = 0;
        store.reach(store[0], 1, &scene, &start);
        require(scene == 4 && start == 100, "reach 1");
        store.reach(store[0], 2, &scene, &start);
        require(scene == kNoScene && start == 300, "reach 2");
        require(store[0].start == 4u && static_cast<u32>(store[0].end - store[0].start) == encoded_size(d), "the encoded size");
        Draft back;
        store.draft(0, back);
        require(back.reach == 2 && back.reach_scene[0] == 4 && back.reach_start[1] == 300 && back.length == 256, "reach in the draft");
        require(store.bytes()[4 + 6] == 4 && store.bytes()[4 + 9] == 2, "the flag and the count follow the header");
        // The edit refusals.
        Draft bad = back;
        bad.length = 4097;  // build 063 (version 12): 4096 steps at the most (1024 until then)
        require(!store.put(0, bad), "4097 steps refused");
        bad = back, bad.reach = 96;
        require(!store.put(0, bad), "a reach of 96 refused");
        bad = back, bad.reach_start[1] = 100;
        require(!store.put(0, bad), "starts must rise");
        bad = back, bad.reach_start[0] = 0;
        require(!store.put(0, bad), "a start of 0 refused");
        bad = back, bad.offset = -200;
        require(!store.put(0, bad), "a window that never reaches its first join refused");
        bad = back, bad.reach_scene[0] = 99;
        require(!store.put(0, bad), "a scene out of range refused");
        // Version 7 bytes: the reach flag and the muted bit mean nothing, are cleared.
        u8 plain[sizeof data];
        const u32 plain_size = as_old(data, n, 7, plain);
        plain[4 + 6] |= 4;
        plain[4 + 9 + 4] |= 0x40;
        require(store.load(plain, plain_size) && store.version() == 12 && store[0].reach == 0 && !store[0].lanes[0].muted &&
                    store.bytes()[2] == 12 && store.bytes()[4 + 6] == 0 && store.bytes()[4 + 9 + 4] == data[4 + 9 + 4],
                "version 7: bits cleared, renumbered");
        // A version 8 store with a damaged reach list is refused.
        require(store.load(data, n), "again");
        store.draft(0, d);
        d.length = 256, d.offset = 100;
        d.reach = 1, d.reach_scene[0] = 4, d.reach_start[0] = 100;
        require(store.put(0, d), "one join");
        u8 buf[kCapacity];
        const u32 size = store.size();
        for (u32 i = 0; i < size; ++i) buf[i] = store.bytes()[i];
        buf[4 + 9] = 0;
        require(!store.load(buf, size), "a reach of 0 behind the flag refused");
        buf[4 + 9] = 32;
        require(!store.load(buf, size), "a reach of 32 refused");
        buf[4 + 9] = 1;
        buf[4 + 10] = 99;
        require(!store.load(buf, size), "a reach scene out of range refused");
        buf[4 + 10] = 4, buf[4 + 11] = 0, buf[4 + 12] = 0;
        require(!store.load(buf, size), "a reach start of 0 refused");
        require(store.load(data, n), "restored");
    }

    // Build 049 (version 9): windows to 1024 steps, a line of its own length (line_length: at most
    // 256 steps, so a point's step is below it), a reach of up to 31 joins; version 8 bytes load as it.
    {
        require(line_length(8) == 8 && line_length(256) == 256 && line_length(257) == 256 && line_length(1024) == 256, "line_length");
        require(store.load(data, n), "back to the tool's bytes");
        Draft d;
        store.draft(0, d);
        for (u32 k = 0; k < 2; ++k) {
            const u32 length = k ? 1024 : 512;
            d.length = length, d.lanes[0].count = 2;
            d.lanes[0].points[0] = Point{0, 128, 0}, d.lanes[0].points[1] = Point{255, 255, 0};
            d.lanes[1].count = 0;
            require(store.put(0, d) && store[0].length == length, "put: a long window");
            Draft back;
            store.draft(0, back);
            require(back.length == length && back.lanes[0].points[1].step == 255, "a long window in the draft");
        }
        // A point's step stays below the line's length: 200 in a 200-step window is out; 1025 steps is out.
        Draft bad = d;
        bad.length = 200;
        bad.lanes[0].points[1].step = 200;
        require(!store.put(0, bad), "a 200-step window's points must be below 200");
        bad = d, bad.length = 4097;
        require(!store.put(0, bad), "4097 refused");
        // A reach of 31 joins, the last start at 4095 (the most until version 12: build 063 allows more, below).
        d.offset = 0;
        d.reach = 31;
        for (u32 j = 0; j < 31; ++j) d.reach_scene[j] = j, d.reach_start[j] = 100 + j;
        d.reach_start[30] = 4095, d.length = 1024;
        require(store.put(0, d) && store[0].reach == 31, "a reach of 31");
        u32 scene = 0, start = 0;
        store.reach(store[0], 31, &scene, &start);
        require(scene == 30 && start == 4095, "reach 31");
        d.reach_start[30] = 4096;
        require(store.put(0, d), "a start of 4096: refused until version 12, fine now");
        d.reach_start[30] = static_cast<unsigned short>(kMaxReachStart + 1);
        require(!store.put(0, d), "a start past kMaxReachStart refused");
        // Version 8 bytes load as version 9.
        require(store.load(data, n), "again");
        store.draft(0, d);
        d.length = 256, d.offset = 100, d.reach = 0;
        require(store.put(0, d), "a 256-step window");
        u8 buf[kCapacity];
        const u32 size = store.size();
        for (u32 i = 0; i < size; ++i) buf[i] = store.bytes()[i];
        u8 old[kCapacity];
        u32 old_size = as_old(buf, size, 8, old);
        require(store.load(old, old_size) && store.version() == 12 && store.bytes()[2] == 12 && store[0].length == 256 &&
                    store.size() == size, "version 8: renumbered, lane C written in");
        old[4 + 4] = 0, old[4 + 5] = 2;  // 512 steps: no version 8 window
        require(!store.load(old, old_size), "version 8 keeps its 256-step limit");
        require(store.load(data, n), "restored");
    }

    // Build 058 (version 10): a backward reach list, after the forward one. "I'd like to be able to have
    // another variation span the entire song, despite it being put at the first instance of scene 2".
    {
        require(store.load(data, n), "the tool's bytes again");
        Draft d;
        store.draft(0, d);
        require(d.back == 0 && store[0].back == 0, "a plain variation: no back list");
        // Pinned at entry 5, from 150 steps before its join to 100 after (250 steps), crossing three joins back
        // (starts 64, 128, 192) and two forward (starts 64 and 90 steps after the join).
        d.entry = 5, d.offset = -200, d.length = 300;
        d.lanes[0].count = 2, d.lanes[0].points[0] = Point{0, 128, 0}, d.lanes[0].points[1] = Point{200, 255, 0};
        d.lanes[1].count = 0;
        d.reach = 2, d.reach_scene[0] = 6, d.reach_scene[1] = kNoScene, d.reach_start[0] = 64, d.reach_start[1] = 90;
        d.back = 3, d.back_scene[0] = 4, d.back_scene[1] = kNoScene, d.back_scene[2] = 2;
        d.back_start[0] = 64, d.back_start[1] = 128, d.back_start[2] = 192;
        require(store.put(0, d) && store[0].back == 3 && store[0].reach == 2, "put: both lists");
        u32 scene = 0, start = 0;
        store.back(store[0], 1, &scene, &start);
        require(scene == 4 && start == 64, "back record 1");
        store.back(store[0], 2, &scene, &start);
        require(scene == kNoScene && start == 128, "back record 2");
        store.back(store[0], 3, &scene, &start);
        require(scene == 2 && start == 192, "back record 3");
        require(store.bytes()[4 + 6] == (kReaching | kBackReaching), "both flags set");
        // The bytes round trip: reload what was saved, the same records, the same bytes.
        u8 buf[kCapacity];
        const u32 size = store.size();
        for (u32 i = 0; i < size; ++i) buf[i] = store.bytes()[i];
        require(store.load(buf, size) && store.size() == size && store[0].back == 3 && store[0].reach == 2, "reloaded");
        Draft again;
        store.draft(0, again);
        bool same_records = again.back == 3 && again.reach == 2;
        for (u32 j = 0; j < 3; ++j) same_records = same_records && again.back_scene[j] == d.back_scene[j] && again.back_start[j] == d.back_start[j];
        for (u32 j = 0; j < 2; ++j) same_records = same_records && again.reach_scene[j] == d.reach_scene[j] && again.reach_start[j] == d.reach_start[j];
        require(same_records, "the records read back as written");
        for (u32 i = 0; i < size; ++i) require(store.bytes()[i] == buf[i], "the same bytes");
        // Validation: starts must rise; the window must start before the last join; scenes, count, range.
        Draft bad = d;
        bad.back_start[1] = 64;
        require(!store.put(0, bad), "back starts that do not rise refused");
        bad = d, bad.back_start[0] = 0;
        require(!store.put(0, bad), "a back start of 0 refused");
        // Build 064: starting exactly on the last recorded entry's start reaches it, so it is accepted now (058 refused it).
        bad = d, bad.offset = -192;
        require(store.put(0, bad), "a window starting exactly on its last back join's entry accepted (build 064)");
        bad = d, bad.offset = -191;
        require(!store.put(0, bad), "a window short of its last back join refused");
        bad = d, bad.offset = -193;
        require(store.put(0, bad), "one step across the last back join accepted");
        bad = d, bad.back_scene[0] = 99;
        require(!store.put(0, bad), "a back scene out of range refused");
        bad = d, bad.back = 96;
        require(!store.put(0, bad), "a back of 96 refused");
        bad = d, bad.back_start[2] = static_cast<unsigned short>(kMaxReachStart + 1), bad.offset = -4095, bad.length = 4096, bad.reach = 0;
        require(!store.put(0, bad), "a back start past kMaxReachStart refused");
        bad = d, bad.back_start[2] = 4095, bad.offset = -4095, bad.length = 4096, bad.reach = 0;
        require(store.put(0, bad), "build 064: a window starting exactly on its last back join's entry accepted, at the new ceiling too");
        bad = d, bad.back_start[2] = 4095, bad.offset = -4094, bad.length = 4095, bad.reach = 0;
        require(!store.put(0, bad), "a window one step short of its last back join refused, at the new ceiling too");
        bad = d, bad.back_start[2] = 4094, bad.offset = -4095, bad.length = 4096, bad.reach = 0;
        require(store.put(0, bad), "one step across a join 4094 back accepted (build 063: past 2047)");
        bad = d, bad.back_start[0] = 100, bad.back_start[1] = 128, bad.back_start[2] = 192;
        require(store.put(0, bad), "other rising starts accepted");
        // The bytes are what held: a refused put changed nothing.
        require(store.load(buf, size) && store.size() == size, "restored");
        // A damaged list in the bytes is refused whole: a flag with a back of 0, 32, scene 99, a start of 0, starts not rising.
        const u32 at = 4 + 9 + 1 + 3 * 2;  // the back list's count byte (after the forward list's two records)
        require(buf[at] == 3, "the back count is where the layout says");
        u8 dmg[kCapacity];
        for (u32 i = 0; i < size; ++i) dmg[i] = buf[i];
        dmg[at] = 0;
        require(!store.load(dmg, size), "a back of 0 behind the flag refused");
        dmg[at] = 32;
        require(!store.load(dmg, size), "a back of 32 refused");
        dmg[at] = 3, dmg[at + 1] = 99;
        require(!store.load(dmg, size), "a back scene out of range refused");
        dmg[at + 1] = 4, dmg[at + 2] = 0, dmg[at + 3] = 0;
        require(!store.load(dmg, size), "a back start of 0 refused");
        dmg[at + 2] = 64, dmg[at + 3] = 0, dmg[at + 5] = 64, dmg[at + 6] = 0;
        require(!store.load(dmg, size), "back starts that do not rise refused");
        // Version 9 bytes (no back list) load as they are, renumbered; the back flag, meaningless then, is cleared.
        store.load(data, n);
        store.draft(0, d);
        d.entry = 5, d.offset = -20, d.length = 100, d.back = 0;
        d.reach = 1, d.reach_scene[0] = 6, d.reach_start[0] = 40;
        require(store.put(0, d), "a version 9 shape");
        const u32 size9 = store.size();
        for (u32 i = 0; i < size9; ++i) buf[i] = store.bytes()[i];
        const u32 old9 = as_old(buf, size9, 9, buf);
        require(store.load(buf, old9) && store.version() == 12 && store.bytes()[2] == 12 && store.size() == size9 &&
                    store[0].back == 0 && store[0].reach == 1 && store[0].offset == -20 && store[0].length == 100,
                "version 9: loads as version 11 with no back list, the reach as it was, lane C written in");
        buf[4 + 6] |= static_cast<u8>(kBackReaching);
        require(store.load(buf, old9) && store[0].back == 0 && store.bytes()[4 + 6] == kReaching, "version 9: the back flag means nothing, cleared");
        buf[2] = 10;
        require(!store.load(buf, old9), "version 10: the same flag with no list behind it is refused");
        // The tool's bytes as version 9 (lane C out, the version byte differs).
        const u32 data9 = as_old(data, n, 9, buf);
        require(store.load(buf, data9) && store.count() == 2 && store[0].back == 0 && store[0].reach == 0 && store.bytes()[2] == 12 && same(data, n),
                "a version 9 store loads as the same bytes with lane C written in");
        require(store.load(data, n), "restored");
    }
    // Build 058: the worst case still fits, or is refused whole. A variation with both lists at 31 records, two
    // 32-point lines, two 15-character names: 9 + 2 * 7 + 2 * (1 + 3 * 31) + 2 * 96 + 2 * 16 = 435 bytes, so 18 fit
    // in kCapacity (24 with the forward list alone). Build 060 (version 11): a third lane makes the worst
    // variation 9 + 3 * 7 + 2 * 94 + 3 * 96 + 3 * 16 = 554 bytes (a maximal lane is 119: 7 + 96 + 16), so 14 fit
    // (8188 / 554 = 14.8); with lane C empty it is 435 + 7 = 442 and 18 still fit (8188 / 442 = 18.5).
    {
        store.clear();
        Draft d = {};
        d.song = 0, d.entry = 40, d.offset = -200, d.length = 1024;
        d.scene_before = 1, d.scene_after = 2;
        d.reach = d.back = 31;
        for (u32 j = 0; j < 31; ++j) {
            d.reach_scene[j] = j, d.reach_start[j] = 100 + j;
            d.back_scene[j] = j, d.back_start[j] = 100 + j;
        }
        for (DraftLane& l : d.lanes) {
            clear_lane(l);
            l.gesture = 0, l.amount = 128, l.target = 0, l.selection = 1, l.count = 32;
            for (u32 k = 0; k < 15; ++k) l.name[k] = 'a';
            l.name[15] = 0;
            for (u32 k = 0; k < 32; ++k) l.points[k] = Point{static_cast<u8>(k * 7), 128, 0};
        }
        require(encoded_size(d) == 9 + 21 + 2 * 94 + 3 * 96 + 3 * 16 && encoded_size(d) == 554, "the worst variation is 554 bytes");
        u32 fitted = 0;
        while (store.put(store.count(), d)) {
            ++fitted;
            require(store.size() <= kCapacity, "never past the capacity");
        }
        require(fitted == 14 && store.size() == 4 + 14 * 554 && store.count() == 14, "14 of the worst fit, the 15th refused");
        // Refused whole: the store is still valid and unchanged, and an edit that grows past the limit is refused too.
        u8 keep[kCapacity];
        const u32 held = store.size();
        for (u32 i = 0; i < held; ++i) keep[i] = store.bytes()[i];
        Draft small = d;
        small.reach = small.back = 0;
        require(store.put(13, small) && store.size() == held - 554 + encoded_size(small), "a smaller one in place");
        require(store.put(13, d) && store.size() == held, "put it back");
        require(store.load(keep, held) && store.count() == 14 && store[13].back == 31 && store[13].reach == 31, "reloads whole");
        require(store[13].lanes[2].count == 32 && store[13].lanes[2].name_length == 15, "lane C reloads whole");
        require(!store.put(14, d) && store.size() == held && store.count() == 14, "the 15th refused, nothing changed");
        for (u32 i = 0; i < held; ++i) require(store.bytes()[i] == keep[i], "a refused put leaves the bytes alone");
        // A growing edit refused at the limit: the room left is 8192 - held bytes; one lane C's worth more than that is refused.
        Draft grow = d;
        grow.lanes[2].count = 0, grow.lanes[2].name[0] = 0;  // 119 - 96 - 16 = lane C at its smallest, 7 bytes
        require(store.put(13, grow) && store.size() == held - 112, "lane C emptied: 112 bytes freed");
        require(store.put(13, d) && store.size() == held, "and filled again");
        for (u32 i = 0; i < held; ++i) require(store.bytes()[i] == keep[i], "the same bytes after the round trip");
        // With lane C empty: two maximal lanes and the empty one, 442 bytes, 18 fit.
        store.clear();
        Draft two = d;
        clear_lane(two.lanes[2]);
        require(encoded_size(two) == 442, "two maximal lanes and an empty C: 442 bytes");
        fitted = 0;
        while (store.put(store.count(), two)) ++fitted;
        require(fitted == 18 && store.size() == 4 + 18 * 442 && store.size() <= kCapacity, "18 of those fit, as before");
        store.load(data, n);
    }

    // Versions 4 and 3: one line a lane (version 3's lanes their gesture's own curve).
    require(store.load(v4, sizeof v4) && store.version() == 12 && store.count() == 2 && same(v6, sizeof v6),
            "version 4: rewritten as version 6, the tool's bytes");
    require(store.load(v3, sizeof v3) && store.version() == 12 && store.count() == 2 && same(v6, sizeof v6),
            "version 3: rewritten as version 6, the tool's bytes");
    require(store[0].lanes[0].curve == 0 && store[0].lanes[1].curve == kSlowStart && store[1].lanes[1].curve == 0,
            "each lane its gesture's own curve");
    {
        // The break made again: its line the leading track's (track 3, the highest), a jump to the bottom.
        Point line[3];
        const u32 k = entry_line(2, 200, 0, 0b111, 32, line);
        p = store.points(store[0], 0, &count);
        require(count == k && p[k - 1].step == line[k - 1].step && p[k - 1].value == 0, "the break made again");
    }
    {
        // Damage in versions 3 and 4 is refused too.
        u8 bad3[sizeof v3];
        for (u32 i = 0; i < sizeof v3; ++i) bad3[i] = v3[i];
        bad3[13] = 19;  // gesture out of range
        require(!store.load(bad3, sizeof bad3) && store.count() == 0, "a damaged version 3 refused");
        require(!store.load(v3, sizeof v3 - 1), "a truncated version 3 refused");
        require(!store.load(v4, sizeof v4 - 1) && store.count() == 0 && !store.kept(), "a truncated version 4 refused");
    }
    {
        // Version 4 rewritten in place: 5 big variations (both lanes custom, 8 tracks x 32 points: 1591
        // bytes each) and 10 small ones, 8189 bytes: as version 5 each big one keeps its lowest track's line.
        static u8 full4[kCapacity];
        u32 at = 4, count4 = 0;
        auto variation = [&](u32 runs, u32 points) {
            const u8 head[9] = {0, 1, 0xe0, 0xff, 32, 0, 0, kNoScene, kNoScene};
            for (u8 b : head) full4[at++] = b;
            for (u32 l = 0; l < 2; ++l) {
                const u8 lane[7] = {static_cast<u8>(runs ? 0 : 6), 128, 16, static_cast<u8>(runs ? 0xff : 1), 2,
                                    static_cast<u8>(runs ? 0 : kEaseOut), static_cast<u8>(runs)};
                for (u8 b : lane) full4[at++] = b;
                for (u32 r = 0; r < runs; ++r) {
                    full4[at++] = static_cast<u8>(r), full4[at++] = static_cast<u8>(points);
                    for (u32 k = 0; k < points; ++k) full4[at++] = static_cast<u8>(k), full4[at++] = static_cast<u8>(9 + r), full4[at++] = 0;
                }
            }
            ++count4;
        };
        while (count4 < 5) variation(8, 32);
        while (count4 < 15) variation(0, 0);
        full4[0] = 'V', full4[1] = 'R', full4[2] = 4, full4[3] = static_cast<u8>(count4);
        require(at == 4 + 5 * 1591 + 10 * 23, "a nearly full version 4 store");
        require(store.load(full4, at) && !store.kept() && store.count() == 15 && store.size() == 4 + 5 * 222 + 10 * 30,
                "rewritten in place, smaller (build 060: each with its empty lane C, 7 bytes more)");
        p = store.points(store[4], 1, &count);
        require(count == 32 && p[31].value == 9 && store[4].lanes[1].selection == 0xff, "each lane its lowest track's line");
        require(store[14].lanes[0].curve == kEaseOut && store[14].lanes[0].gesture == 6, "the small ones as they were");
    }
    // Versions 2 and 1: rewritten as version 6 as they load, as the tool does.
    require(store.load(legacy2, sizeof legacy2) && store.version() == 12 && same(upgraded, sizeof upgraded),
            "version 2: the tool's upgrade");
    const Variation& u = store[0];
    require(u.lanes[0].gesture == 1 && u.lanes[0].target == kVolume && u.lanes[0].selection == 0b111 &&
            u.lanes[0].default_tracks && u.lanes[0].default_target, "a build: lane a, on the volume");
    p = store.points(u, 0, &count);
    require(p && count >= 2 && p[0].value == 0 && p[count - 1].value == 128, "level 0..255 as 0..128 (as set), made again");
    require(u.lanes[1].gesture == 0 && u.lanes[1].target == kTargetNotes && u.lanes[1].selection == 0b10,
            "its notes line: lane b, custom, on the tracks with points");
    require(store[1].lanes[0].target == kCutoff && store[1].lanes[1].gesture == kNoGesture, "a filter line alone: lane b empty");
    require(store.load(legacy1, sizeof legacy1) && store.version() == 12 && store.count() == 2 &&
            store[0].scene_before == kNoScene && store[0].lanes[0].target == kVolume, "version 1 too");

    // Build 060 (version 11): the version 1 and 2 upgrade writes its intermediate two lanes a variation (store.hpp
    // kLanesV10, not kLanes, which is 3), so old lines go A, B, then a variation of their own for a third and
    // fourth, and lane C is empty in every one. (With kLanes in that code the pair's third lane read a line
    // that was not there, and the intermediate was three lanes long for the reader's two.)
    for (u32 version = 1; version <= 2; ++version) {
        const u8* in = version == 2 ? legacy4v2 : legacy4v1;
        const u32 in_size = version == 2 ? sizeof legacy4v2 : sizeof legacy4v1;
        require(store.load(in, in_size) && store.version() == 12 && store.count() == 4 && !store.kept(), "legacy lines: four variations");
        if (version == 2) require(same(upgraded4, sizeof upgraded4), "version 2: the tool's upgrade");
        // First variation: level, notes (A, B); the second: send, filter; the third (another variation): level, send; the fourth: filter.
        const u32 want_a[4] = {kVolume, kSend, kVolume, kCutoff}, want_b[4] = {kTargetNotes, kCutoff, kSend, kNoTarget};
        for (u32 i = 0; i < 4; ++i) {
            const Variation& v = store[i];
            require(v.lanes[0].target == want_a[i] && v.lanes[0].gesture == 0, "legacy lines: lane A holds the first of a pair");
            require(v.lanes[1].target == want_b[i] && v.lanes[1].gesture == (want_b[i] == kNoTarget ? kNoGesture : 0u), "legacy lines: lane B the second, or empty");
            require(v.lanes[2].gesture == kNoGesture && v.lanes[2].target == kNoTarget && v.lanes[2].selection == 0 &&
                        v.lanes[2].count == 0 && v.lanes[2].name_length == 0 && v.lanes[2].default_target,
                    "legacy lines: lane C empty");
        }
        require(store[0].song == 1 && store[1].song == 1 && store[0].offset == -32 && store[1].offset == -32 && store[2].song == 0 && store[3].song == 0 &&
                    store[2].offset == 16 && store[0].lanes[0].selection == 0b1 && store[0].lanes[1].selection == 0b10 &&
                    store[1].lanes[0].selection == 0b100 && store[1].lanes[1].selection == 0b1000,
                "legacy lines: each pair on its own window and tracks");
        u32 count;
        const Point* p = store.points(store[1], 1, &count);
        require(p && count == 2 && p[0].step == 8 && p[1].step == 20, "legacy lines: the fourth line's points in lane B of the second");
        // And the store is a valid version 11 store: saved bytes load back the same.
        u8 again[kCapacity];
        const u32 again_size = store.size();
        for (u32 i = 0; i < again_size; ++i) again[i] = store.bytes()[i];
        require(again[2] == 12 && store.load(again, again_size) && same(again, again_size) && store.count() == 4, "legacy lines: round trips");
    }
    // Version 3 and 4 hold two lanes too: lane C empty after their rewrite (v6 above is the whole store's bytes).
    require(store.load(v4, sizeof v4) && store[0].lanes[2].gesture == kNoGesture && store[1].lanes[2].count == 0 &&
                store[0].lanes[0].gesture == 2 && store[0].lanes[1].gesture == 4,
            "version 4: lanes A and B as they were, lane C empty");

    // Build 060: version 10's own bytes (two lanes a variation) load with lane C empty, and lanes A and B behave exactly
    // as they did: the same fields, the same lines, and the whole store the version 11 bytes the tool writes.
    {
        require(store.load(data10, sizeof data10) && store.version() == 12 && store.count() == 2 && !store.kept(), "version 10 loads");
        require(store.size() == sizeof data10 + 2 * kLaneHeader && same(data, n), "as the tool's version 11 bytes, a lane C written in each");
        Store eleven;
        require(eleven.load(data, n), "the version 11 bytes");
        for (u32 i = 0; i < 2; ++i)
            for (u32 l = 0; l < 2; ++l) {
                const Lane& a = store[i].lanes[l];
                const Lane& b = eleven[i].lanes[l];
                require(a.gesture == b.gesture && a.amount == b.amount && a.target == b.target && a.selection == b.selection &&
                            a.curve == b.curve && a.rate == b.rate && a.muted == b.muted && a.count == b.count && a.default_tracks == b.default_tracks,
                        "version 10: lanes A and B the same");
            }
        require(store[0].lanes[0].gesture == 2 && store[0].lanes[0].target == 15 && store[0].lanes[1].gesture == 4 && store[0].lanes[1].target == 11,
                "version 10: lane a a break on the volume, lane b a rise on the release");
        for (u32 i = 0; i < 2; ++i) {
            const Lane& c = store[i].lanes[2];
            require(c.gesture == kNoGesture && c.target == kNoTarget && c.selection == 0 && c.count == 0 && c.name_length == 0 && !c.muted &&
                        c.default_target && c.rate == 1 && c.curve == 0,
                    "version 10: lane C empty (no gesture, no target, no tracks, no points)");
        }
        // An empty lane C plays nothing, as an empty lane B does: the variation's targets are lane A's and lane B's alone.
        SongPosition pos = {};
        pos.playing = true, pos.song = 1, pos.entry = 1, pos.next_entry = 2, pos.position = 40 * 480, pos.length = 64 * 480;
        pos.scene = kNoScene, pos.scene_before = kNoScene;
        Targets with_c, without_c;
        targets(store, pos, &with_c);
        targets(eleven, pos, &without_c);
        require(with_c.count == without_c.count && with_c.count > 0, "version 10: the same targets as the version 11 store");
        for (u32 k = 0; k < with_c.count; ++k)
            require(with_c.drive[k].track == without_c.drive[k].track && with_c.drive[k].word == without_c.drive[k].word &&
                        with_c.drive[k].value == without_c.drive[k].value,
                    "version 10: the same drives");
        // A version 10 store is also a draft with an empty lane C, which stays valid.
        Draft d;
        store.draft(0, d);
        require(d.lanes[2].gesture == kNoGesture && d.lanes[2].count == 0 && Store::valid(d) && store.put(0, d) && same(data, n), "its draft with lane C empty puts back");
        // Too full for the lanes to be written in: kept as it is (not played, not edited), like an old store too full to rewrite.
        static u8 fat[kCapacity];
        Store scratch;
        Draft big = {};
        big.song = 0, big.entry = 1, big.offset = 0, big.length = 32, big.scene_before = big.scene_after = kNoScene;
        for (DraftLane& l : big.lanes) clear_lane(l);
        for (u32 n2 = 0; n2 < 2; ++n2) {
            DraftLane& l = big.lanes[n2];
            l.gesture = 0, l.target = kVolume, l.selection = 1, l.count = 32;
            for (u32 k = 0; k < 32; ++k) l.points[k] = Point{static_cast<u8>(k), 128, 0};
        }
        while (scratch.put(scratch.count(), big)) {
        }
        // 36 two-lane variations of 222 bytes (lane C included) leave 4 + 36 * 222 = 7996; as version 10 they were 215.
        u32 fat_size = as_old(scratch.bytes(), scratch.size(), 10, fat);
        require(fat_size <= kCapacity && store.load(fat, fat_size) && !store.kept() && store.count() == scratch.count(), "a full version 10 store loads");
        // Fill the old bytes to the brim with trailing small variations: 8192 bytes of version 10 cannot take 7 more each.
        u32 at = fat_size;
        u32 added = 0;
        while (at + 9 + 2 * 7 <= kCapacity && fat[3] < kMaxVariations) {
            const u8 small[9 + 14] = {0, 1, 0xe0, 0xff, 8, 0, 0, kNoScene, kNoScene, 0, 128, 15, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0};
            for (u8 b : small) fat[at++] = b;
            ++fat[3], ++added;
        }
        require(at > kCapacity - 23 && at <= kCapacity && added > 0, "a version 10 store to the brim");
        require(store.load(fat, at) && store.kept() && store.count() == 0 && store.size() == at && store.version() == 10, "too full for lane C: kept as it is, version 10");
        require(store.bytes()[2] == 10 && store.bytes()[at - 1] == fat[at - 1], "saved back unchanged");
        Draft none = {};
        none.length = 16, none.offset = -16;
        for (DraftLane& l : none.lanes) clear_lane(l);
        require(!store.put(0, none), "and not edited");
        require(store.load(data, n), "restored");
    }

    // Build 060: all three lanes through the store's bytes: each lane's gesture, target, tracks, curve, rate, mute, name and
    // line come back as put, in order A, B, C, and so do the bytes.
    {
        Draft d = {};
        d.song = 3, d.entry = 7, d.offset = -16, d.length = 48, d.scene_before = 2, d.scene_after = 3;
        for (DraftLane& l : d.lanes) clear_lane(l);
        DraftLane& a = d.lanes[0];
        a.gesture = 4, a.amount = 100, a.target = kCutoff, a.selection = 0x03, a.curve = 20, a.count = 2, a.default_target = false;
        a.points[0] = Point{0, 128, 0}, a.points[1] = Point{47, 250, -10};
        DraftLane& b = d.lanes[1];
        b.gesture = 0, b.amount = 77, b.target = 1, b.selection = 0x10, b.rate = 4, b.count = 3, b.default_target = false, b.edited = true;
        b.name[0] = 't', b.name[1] = 'o', b.name[2] = 'n', b.name[3] = 'e', b.name[4] = 0;
        b.points[0] = Point{0, 0, 0}, b.points[1] = Point{20, 255, 5}, b.points[2] = Point{40, 3, 0};
        DraftLane& c = d.lanes[2];
        c.gesture = 12, c.amount = 200, c.target = kSend, c.selection = 0xc0, c.curve = -33, c.rate = 2, c.muted = true, c.count = 2;
        c.default_target = false, c.default_tracks = true;
        c.points[0] = Point{0, 128, 0}, c.points[1] = Point{30, 10, 99};
        require(Store::valid(d), "three lanes are a valid variation");
        static Store s1, s2;
        s1.clear();
        require(s1.put(0, d) && s1.size() == 4 + encoded_size(d) && encoded_size(d) == 9 + 3 * 7 + 3 * (2 + 3 + 2) + 5, "put: three lanes");
        // Lane C comes after lane B exactly as B after A: its header starts 7 + 3 * 3 bytes after B's start (B has a name: 1 + 4).
        const u32 lane_a = 4 + 9, lane_b = lane_a + 7 + 3 * 2, lane_c = lane_b + 7 + 5 + 3 * 3;
        require(s1.bytes()[lane_a] == 4 && s1.bytes()[lane_b] == 0 && s1.bytes()[lane_c] == 12 && s1.bytes()[lane_c + 1] == 200 &&
                    s1.bytes()[lane_c + 2] == kSend && s1.bytes()[lane_c + 3] == 0xc0,
                "the bytes: lane A, lane B, lane C in order");
        require(s1.bytes()[lane_c + 4] == (1 | (1 << 4) | kLaneMuted) && static_cast<signed char>(s1.bytes()[lane_c + 5]) == -33 && s1.bytes()[lane_c + 6] == 2,
                "lane C's flags (default tracks, rate 2, muted), curve and point count");
        u8 buf[kCapacity];
        const u32 size = s1.size();
        for (u32 i = 0; i < size; ++i) buf[i] = s1.bytes()[i];
        require(s2.load(buf, size) && s2.count() == 1 && s2.size() == size, "reloads");
        Draft back;
        s2.draft(0, back);
        for (u32 l = 0; l < kLanes; ++l) {
            const DraftLane& x = d.lanes[l];
            const DraftLane& y = back.lanes[l];
            require(x.gesture == y.gesture && x.amount == y.amount && x.target == y.target && x.selection == y.selection && x.curve == y.curve &&
                        x.rate == y.rate && x.muted == y.muted && x.default_tracks == y.default_tracks && x.default_target == y.default_target &&
                        x.edited == y.edited && x.count == y.count && !std::strcmp(x.name, y.name),
                    "round trip: every lane's fields");
            for (u32 k = 0; k < x.count; ++k)
                require(x.points[k].step == y.points[k].step && x.points[k].value == y.points[k].value && x.points[k].curve == y.points[k].curve,
                        "round trip: every lane's points");
        }
        require(s2.put(0, back), "put back");
        for (u32 i = 0; i < size; ++i) require(s2.bytes()[i] == buf[i], "round trip: the same bytes");
        // Damage in lane C is refused whole: its gesture, its target, and a truncation.
        buf[lane_c] = 19;
        require(!s2.load(buf, size), "lane C: a gesture out of range refused");
        buf[lane_c] = 12, buf[lane_c + 2] = 12;
        require(!s2.load(buf, size), "lane C: a target out of range refused");
        buf[lane_c + 2] = kSend;
        require(!s2.load(buf, size - 1), "lane C truncated: refused");
        require(s2.load(buf, size), "restored");
        // The lanes the page edits: one variation's lane C touched alone leaves A and B as they were.
        Draft e = back;
        e.lanes[2].amount = 1;
        require(s2.put(0, e), "lane C edited");
        Draft after;
        s2.draft(0, after);
        require(after.lanes[2].amount == 1 && after.lanes[0].amount == 100 && after.lanes[1].amount == 77, "lane A and B untouched");
        // Fixed target lanes: kLanes variations with the version 11 bytes the tool writes decode alike (tests/test_variations_tool.py).
        require(s2.load(data, n), "restored");
    }

    // Older bytes too full to rewrite: kept as they are, saved back, nothing played or edited.
    {
        static u8 full[kCapacity];
        // A version 2 store near full: 38 variations with two 32-point lines (210 bytes each, 219
        // as version 4, the upgrade's way to version 5) and ten small ones (19 bytes, 28 as version
        // 4): 8174 bytes, 8606 as version 4, too many on the way.
        u32 at = 4, count2 = 0;
        auto variation = [&](u32 lines, u32 points) {
            full[at++] = 0, full[at++] = 1, full[at++] = 0xe0, full[at++] = 0xff, full[at++] = 32, full[at++] = 0;
            full[at++] = 0, full[at++] = 0, full[at++] = 1, full[at++] = 0, full[at++] = 0;
            full[at++] = kNoScene, full[at++] = kNoScene, full[at++] = static_cast<u8>(lines);
            for (u32 l = 0; l < lines; ++l) {
                full[at++] = static_cast<u8>(l << 4), full[at++] = static_cast<u8>(points);
                for (u32 k = 0; k < points; ++k) full[at++] = static_cast<u8>(k), full[at++] = 255, full[at++] = 0;
            }
            ++count2;
        };
        while (at + 14 + 2 * (2 + 96) <= kCapacity && count2 < 60) variation(2, 32);
        while (at + 14 + 2 + 3 <= kCapacity && count2 < 64) variation(1, 1);
        full[0] = 'V', full[1] = 'R', full[2] = 2, full[3] = static_cast<u8>(count2);
        require(count2 == 48 && at == 8174, "a nearly full version 2 store");
        require(store.load(full, at) && store.kept() && store.count() == 0 && store.version() == 2, "kept as it is");
        require(store.size() == at && store.bytes()[at - 1] == full[at - 1], "saved back unchanged");
        Draft d = {};
        d.length = 16, d.offset = -16;
        clear_lane(d.lanes[0]), clear_lane(d.lanes[1]), clear_lane(d.lanes[2]);
        require(!store.put(0, d), "and not edited");
        full[5] = 99;  // entry out of range
        require(!store.load(full, at) && !store.kept() && store.size() == 0, "a damaged one is still refused");
    }
    u32 scene_out = 0, start_out = 0;
    // Build 063 (format version 12: "I'd like a song spanning variation to be an option"): windows to 4096 steps,
    // 95 joins each way, offsets to 8191, starts to kMaxReachStart. The lists at their new maximum, round tripped
    // through the store's bytes; what is refused beyond them; version 11 bytes read within their own ranges; the budget.
    {
        require(kVersion == 12 && kMaxLength == 4096 && kMaxReach == 95 && kMaxOffset == 8191 && kMaxReachStart == 12286, "the version 12 ceilings");
        require(sizeof(Draft::reach_start[0]) == 2 && sizeof(Draft::reach_scene[0]) == 1 && sizeof(Draft::back_start[0]) == 2 && sizeof(Draft::back_scene[0]) == 1 &&
                    kMaxReachStart <= 0xffff && kMaxOffset <= 32767,
                "Draft's records are a u8 and a u16, which hold the ranges (and the stored i16 offset holds kMaxOffset)");
        auto base = [](Draft& d) {
            d = Draft{};
            d.song = 0, d.entry = 40, d.scene_before = 1, d.scene_after = 2;
            for (DraftLane& l : d.lanes) clear_lane(l);
            d.lanes[0].gesture = 0, d.lanes[0].amount = 128, d.lanes[0].target = kVolume, d.lanes[0].selection = 1, d.lanes[0].count = 2;
            d.lanes[0].points[0] = Point{0, 0, 0}, d.lanes[0].points[1] = Point{7, 255, 0};  // within the shortest window's line
        };
        // Both lists at 95 records, the longest window; every other scene unknown.
        Draft both;
        base(both);
        both.offset = -3900, both.length = 4096, both.reach = both.back = 95;
        for (u32 j = 0; j < 95; ++j) {
            both.reach_scene[j] = j % 3 ? static_cast<u8>(j) : kNoScene, both.reach_start[j] = static_cast<unsigned short>(40 * (j + 1));
            both.back_scene[j] = j % 5 ? static_cast<u8>(98 - j % 98) : kNoScene, both.back_start[j] = static_cast<unsigned short>(40 * (j + 1));
        }
        // Forward to the furthest start: the longest window from the latest offset, its last join at kMaxReachStart.
        Draft far;
        base(far);
        far.offset = static_cast<i32>(kMaxOffset), far.length = kMaxLength, far.reach = 95;
        for (u32 j = 0; j < 95; ++j) far.reach_scene[j] = static_cast<u8>(j), far.reach_start[j] = static_cast<unsigned short>(100 + j);
        far.reach_start[94] = static_cast<unsigned short>(kMaxReachStart);
        // Back to the furthest start: from the earliest offset, its last join one step across (8190).
        Draft deep;
        base(deep);
        deep.offset = -static_cast<i32>(kMaxOffset), deep.length = kMaxLength, deep.back = 95;
        for (u32 j = 0; j < 95; ++j) deep.back_scene[j] = static_cast<u8>(j), deep.back_start[j] = static_cast<unsigned short>(100 + j);
        deep.back_start[94] = static_cast<unsigned short>(kMaxOffset - 1);
        store.clear();
        require(Store::valid(both) && Store::valid(far) && Store::valid(deep), "the three maximal drafts are valid");
        require(encoded_size(both) == 9 + 21 + 2 * (1 + 3 * 95) + 6 && encoded_size(both) == 9 + 21 + 572 + 6, "both lists at 95: 572 bytes of records");
        require(store.put(0, both) && store.put(1, far) && store.put(2, deep) && store.count() == 3, "95 records each way put");
        require(store[0].reach == 95 && store[0].back == 95 && store[1].reach == 95 && store[2].back == 95 && store[0].length == 4096, "counts and length held");
        u8 kept[kCapacity];
        const u32 held = store.size();
        for (u32 i = 0; i < held; ++i) kept[i] = store.bytes()[i];
        require(kept[2] == 12, "saved as version 12");
        require(store.load(kept, held) && store.count() == 3 && store.size() == held, "the bytes load back");
        for (u32 i = 0; i < held; ++i) require(store.bytes()[i] == kept[i], "the same bytes after a reload");
        Draft got;
        for (u32 k = 0; k < 3; ++k) {
            const Draft& want = k == 0 ? both : k == 1 ? far : deep;
            store.draft(k, got);
            bool ok = got.reach == want.reach && got.back == want.back && got.length == want.length && got.offset == want.offset;
            for (u32 j = 0; j < want.reach; ++j) ok = ok && got.reach_scene[j] == want.reach_scene[j] && got.reach_start[j] == want.reach_start[j];
            for (u32 j = 0; j < want.back; ++j) ok = ok && got.back_scene[j] == want.back_scene[j] && got.back_start[j] == want.back_start[j];
            for (u32 j = want.reach; j < kMaxReach; ++j) ok = ok && got.reach_scene[j] == kNoScene && got.reach_start[j] == 0;
            for (u32 j = want.back; j < kMaxReach; ++j) ok = ok && got.back_scene[j] == kNoScene && got.back_start[j] == 0;
            require(ok, "every record reads back as written");
        }
        store.reach(store[1], 95, &scene_out, &start_out);
        require(scene_out == 94 && start_out == kMaxReachStart, "the last forward record: 12286");
        store.back(store[2], 95, &scene_out, &start_out);
        require(scene_out == 94 && start_out == kMaxOffset - 1, "the last backward record: 8190");

        // Beyond the maximum: refused whole, the bytes unchanged.
        auto refused = [&](const Draft& bad, const char* why) {
            require(!Store::valid(bad) && !store.put(0, bad), why);
            for (u32 i = 0; i < held; ++i) require(store.bytes()[i] == kept[i], "a refused put leaves the bytes alone");
        };
        Draft bad = both;
        bad.reach = 96;
        refused(bad, "a reach of 96 refused");
        bad = both, bad.back = 96;
        refused(bad, "a back of 96 refused");
        bad = far, bad.reach_start[94] = static_cast<unsigned short>(kMaxReachStart + 1);
        refused(bad, "a reach start above kMaxReachStart refused");
        bad = deep, bad.back_start[94] = static_cast<unsigned short>(kMaxReachStart + 1);
        refused(bad, "a back start above kMaxReachStart refused");
        bad = both, bad.length = 4097;
        refused(bad, "a length above 4096 refused");
        bad = both, bad.offset = static_cast<i32>(kMaxOffset) + 1;
        refused(bad, "an offset above 8191 refused");
        bad = both, bad.offset = -static_cast<i32>(kMaxOffset) - 1;
        refused(bad, "an offset below -8191 refused");
        // The same on load: a version 12 store with a count of 96, or a start above the maximum, is no store.
        u8 broken[kCapacity];
        for (u32 i = 0; i < held; ++i) broken[i] = kept[i];
        broken[4 + 9] = 96;
        require(!store.load(broken, held), "a reach count of 96 in the bytes refused");
        for (u32 i = 0; i < held; ++i) broken[i] = kept[i];
        const u32 last_reach = store[1].reach_at + 3 * 94;
        broken[last_reach + 1] = static_cast<u8>((kMaxReachStart + 1) & 0xff), broken[last_reach + 2] = static_cast<u8>((kMaxReachStart + 1) >> 8);
        require(!store.load(broken, held), "a reach start above the maximum in the bytes refused");
        require(store.load(kept, held), "restored");

        // Version 11 bytes: the fixture loads unchanged but for the number; their own ranges hold.
        require(store.load(data11, sizeof data11) && store.count() == 2 && !store.kept() && store.version() == 12 && store.bytes()[2] == 12 && same(data, n),
                "a version 11 fixture loads as the same bytes, version 12");
        require(store.size() == sizeof data11, "no lane to write in: the size does not change");
        // A version 11 store as full as the budget allows loads whole (it does not grow, so cannot be kept).
        Draft w31;
        base(w31);
        w31.offset = -200, w31.length = 1024, w31.reach = w31.back = 31;
        for (u32 j = 0; j < 31; ++j) {
            w31.reach_scene[j] = w31.back_scene[j] = static_cast<u8>(j);
            w31.reach_start[j] = w31.back_start[j] = static_cast<unsigned short>(100 + j);
        }
        for (DraftLane& l : w31.lanes) {
            l.gesture = 0, l.amount = 128, l.target = 0, l.selection = 1, l.count = 32;
            for (u32 k = 0; k < 15; ++k) l.name[k] = 'a';
            l.name[15] = 0;
            for (u32 k = 0; k < 32; ++k) l.points[k] = Point{static_cast<u8>(k * 7), 128, 0};
        }
        store.clear();
        while (store.put(store.count(), w31)) {
        }
        require(store.count() == 14 && store.size() == 4 + 14 * 554, "a full store of version 11's worst variations");
        u8 full11[kCapacity];
        const u32 full_size = store.size();
        for (u32 i = 0; i < full_size; ++i) full11[i] = store.bytes()[i];
        full11[2] = 11;
        require(store.load(full11, full_size) && store.count() == 14 && !store.kept() && store.size() == full_size && store.bytes()[2] == 12,
                "a full version 11 store loads whole and is not kept");
        // Version 11 keeps its ranges: 32 records, a start of 4096, a window of 2048 or an offset of 2048 are no version 11.
        store.clear();
        Draft v;
        base(v);
        v.offset = 0, v.length = 1024, v.reach = 32;
        for (u32 j = 0; j < 32; ++j) v.reach_scene[j] = static_cast<u8>(j), v.reach_start[j] = static_cast<unsigned short>(10 + j);
        require(store.put(0, v), "32 records are version 12's");
        u8 b11[kCapacity];
        auto as11 = [&](const Draft& d12, const char* why_not) {
            store.clear();
            require(store.put(0, d12), why_not);
            const u32 size = store.size();
            for (u32 i = 0; i < size; ++i) b11[i] = store.bytes()[i];
            b11[2] = 11;
            require(!store.load(b11, size), why_not);
        };
        as11(v, "version 11: 32 reach records refused");
        v.reach = 1, v.reach_start[0] = 4096, v.length = 1024, v.offset = 3500;
        as11(v, "version 11: a reach start of 4096 refused");
        base(v);
        v.length = 2048;
        as11(v, "version 11: a window of 2048 steps refused");
        base(v);
        v.offset = 2048, v.length = 64;
        as11(v, "version 11: an offset of 2048 refused");
        base(v);
        v.offset = -2048, v.length = 64;
        as11(v, "version 11: an offset of -2048 refused");
        base(v);
        v.offset = -300, v.length = 100, v.back = 32;
        for (u32 j = 0; j < 32; ++j) v.back_scene[j] = static_cast<u8>(j), v.back_start[j] = static_cast<unsigned short>(10 + j);
        v.offset = -400;
        as11(v, "version 11: 32 back records refused");
        require(store.load(data, n), "restored");
    }

    // Build 063: the budget at the new ceiling. A variation that spans a 96-entry song from the middle (55 joins forward, 39
    // back: every join the song has but the nearest) with one line and two empty lanes costs 9 + 2 + 3 * 94 + 3 * 7 + 6
    // = 320 bytes; the largest the format allows (both lists at 95, three maximal lanes, 15-character names) 938. Either way
    // the store takes what fits and refuses the rest whole.
    {
        Draft d = {};
        d.song = 0, d.entry = 40, d.scene_before = 1, d.scene_after = 2, d.offset = -1280, d.length = 3072;
        for (DraftLane& l : d.lanes) clear_lane(l);
        d.lanes[0].gesture = 0, d.lanes[0].amount = 128, d.lanes[0].target = kVolume, d.lanes[0].selection = 1, d.lanes[0].count = 2;
        d.lanes[0].points[0] = Point{0, 0, 0}, d.lanes[0].points[1] = Point{255, 255, 0};
        d.reach = 55, d.back = 39;
        for (u32 j = 0; j < 55; ++j) d.reach_scene[j] = static_cast<u8>(j), d.reach_start[j] = static_cast<unsigned short>(32 * (j + 1));
        for (u32 j = 0; j < 39; ++j) d.back_scene[j] = static_cast<u8>(j), d.back_start[j] = static_cast<unsigned short>(32 * (j + 1));
        require(Store::valid(d) && encoded_size(d) == 9 + 2 + 3 * 94 + 3 * 7 + 6 && encoded_size(d) == 320, "a song-spanning variation is 320 bytes");
        store.clear();
        u32 fitted = 0;
        while (store.put(store.count(), d)) {
            ++fitted;
            require(store.size() <= kCapacity, "never past the capacity");
        }
        require(fitted == (kCapacity - 4) / 320 && fitted == 25 && store.size() == 4 + 25 * 320, "25 song-spanning variations fit, the 26th refused");
        // The worst: both lists at 95, three maximal lanes.
        Draft worst = d;
        worst.offset = -3900, worst.length = 4096, worst.reach = worst.back = 95;
        for (u32 j = 0; j < 95; ++j) {
            worst.reach_scene[j] = worst.back_scene[j] = static_cast<u8>(j);
            worst.reach_start[j] = worst.back_start[j] = static_cast<unsigned short>(40 * (j + 1));
        }
        for (DraftLane& l : worst.lanes) {
            l.gesture = 0, l.amount = 128, l.target = 0, l.selection = 1, l.count = 32;
            for (u32 k = 0; k < 15; ++k) l.name[k] = 'a';
            l.name[15] = 0;
            for (u32 k = 0; k < 32; ++k) l.points[k] = Point{static_cast<u8>(k * 7), 128, 0};
        }
        require(Store::valid(worst) && encoded_size(worst) == 9 + 21 + 2 * (1 + 3 * 95) + 3 * 96 + 3 * 16 && encoded_size(worst) == 938, "the worst variation is 938 bytes");
        store.clear();
        fitted = 0;
        while (store.put(store.count(), worst)) {
            ++fitted;
            require(store.size() <= kCapacity, "never past the capacity (worst)");
        }
        require(fitted == 8 && store.size() == 4 + 8 * 938 && store.count() == 8, "8 of the worst fit, the 9th refused");
        u8 keep[kCapacity];
        const u32 now = store.size();
        for (u32 i = 0; i < now; ++i) keep[i] = store.bytes()[i];
        require(!store.put(8, worst) && store.size() == now && store.count() == 8, "the 9th refused, nothing changed");
        for (u32 i = 0; i < now; ++i) require(store.bytes()[i] == keep[i], "a refused put leaves the bytes alone");
        // A growing edit at the limit is refused too: variation 0 grows from a spanning one to the worst.
        store.clear();
        for (u32 i = 0; i < 25; ++i) require(store.put(i, d), "filling");
        const u32 before = store.size();
        for (u32 i = 0; i < before; ++i) keep[i] = store.bytes()[i];
        require(!store.put(3, worst) && store.size() == before, "growing 320 to 938 bytes with 192 spare refused");
        for (u32 i = 0; i < before; ++i) require(store.bytes()[i] == keep[i], "and the bytes are as they were");
        require(store.load(keep, before) && store.count() == 25 && store[24].reach == 55 && store[24].back == 39, "reloads whole");
        require(store.load(data, n), "restored");
    }
    std::puts("variations store: ok");
    return 0;
}
