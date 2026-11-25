// Host test (build 025): engine controls by slot ("param 1".."param 4") and by
// name (engine.hpp), as the lanes play them (window.hpp targets), and their
// storage (format version 6: a lane's name after its curve, flags bit 3;
// version 5 read as version 6).
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
// Four engines: track 1 a synth (tone, env, shape, tremolo), track 2 another
// (pitch, tone, drive, air, and "glide" in slot 6), track 3 a drum whose slots
// stock leaves unnamed (the SDK's "m1 N": build 034, placeholders in slots 1-4 too), track 4 a sampler (start, tune, env, end), track
// 5 an engine naming its fourth "play mode"; track 6 no engine controls
// (all refused); fixed labels on every track. `readable` false: nothing (outside
// a callback).
static bool readable = true;
static bool label(u32 track, u32 word, char* text, u32 capacity) {
    text[0] = 0;
    if (!readable || track >= kTracks) return false;
    const opxy::extensions::TrackTarget* t = opxy::extensions::track_target(word);
    if (!t) return false;
    const char* name = t->label;
    if (!name) {
        static const char* const engines[6][8] = {
            {"tone", "env", "shape", "tremolo", nullptr, nullptr, nullptr, nullptr},
            {"pitch", "tone", "drive", "air", nullptr, "glide", nullptr, nullptr},
            {"m1 1", "m1 2", "m1 3", "m1 4", nullptr, nullptr, nullptr, nullptr},
            {"start", "tune", "env", "end", nullptr, nullptr, nullptr, nullptr},
            {"color", "tone", "decay", "play mode", nullptr, nullptr, nullptr, nullptr},
            {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
        };
        name = track < 6 ? engines[track][word] : "x";
        if (!name) return false;
    }
    std::strncpy(text, name, capacity - 1);
    text[capacity - 1] = 0;
    return true;
}

int main() {
    // What counts as a name.
    require(engine::unnamed("") && engine::unnamed("param1") && engine::unnamed("param 4") && engine::unnamed("m1 3"),
            "placeholders are no names");
    require(!engine::unnamed("tone") && !engine::unnamed("param") && !engine::unnamed("parameter") && !engine::unnamed("m1 9"),
            "real names are names");
    // Build 032: parameter 4 is a control like any other (025-031 kept play mode, end and a
    // percussion track's parameter 4 off the list).
    require(engine::safe(0, 3, "play mode", 0) && engine::safe(0, 3, "end", 0) && engine::safe(2, 3, "reverse", 0x04),
            "parameter 4 is offered: play mode, end, a drum's reverse");

    // Where each lane target lands, track by track.
    Engines e = {label, 0};
    // param 1..4: the slot on every track that names it.
    require(engine::word(&e, 0, 0, "") == 0 && engine::word(&e, 1, 0, "") == 0 && engine::word(&e, 3, 0, "") == 0,
            "param 1 on the synths and the sampler");
    require(engine::word(&e, 2, 0, "") == 0 && engine::word(&e, 2, 1, "") == 1 && engine::word(&e, 2, 3, "") == 3,
            "on the drum's unnamed controls too (build 034; 025-033 left them alone)");
    require(engine::word(&e, 5, 0, "") < 0, "not where the engine has no control");
    require(engine::word(&e, 0, 3, "") == 3 && engine::word(&e, 3, 3, "") == 3 && engine::word(&e, 4, 3, "") == 3,
            "param 4 on the synth, the sampler's end and a play mode (build 032)");
    e.percussion = 0x01;
    require(engine::word(&e, 0, 3, "") == 3, "and on a track heard as percussion (build 032)");
    e.percussion = 0;
    // A name: wherever each engine has it.
    require(engine::word(&e, 0, 1, "tone") == 0 && engine::word(&e, 1, 1, "tone") == 1 && engine::word(&e, 4, 1, "tone") == 1,
            "tone: slot 1 on track 1, slot 2 on tracks 2 and 5, whatever slot it was chosen in");
    require(engine::word(&e, 3, 1, "tone") < 0 && engine::word(&e, 2, 1, "tone") < 0, "left alone where the engine has none");
    require(engine::word(&e, 0, 2, "env") == 1 && engine::word(&e, 3, 2, "env") == 2, "env: track 1's slot 2, the sampler's 3");
    require(engine::word(&e, 1, 5, "glide") == 5 && engine::word(&e, 0, 5, "glide") < 0, "past slot 4 by name");
    require(engine::word(&e, 3, 3, "end") == 3 && engine::word(&e, 4, 3, "play mode") == 3, "end and play mode by name too (build 032)");
    // No names to read: by slot as before 025, nothing by name.
    readable = false;
    require(engine::word(&e, 2, 0, "") == 0 && engine::word(&e, 0, 1, "tone") < 0, "unreadable: the slot; a name nothing");
    readable = true;
    require(engine::word(nullptr, 2, 0, "") == 0 && engine::word(nullptr, 0, 0, "tone") < 0, "no Engines: the slot; a name nothing");
    require(engine::word(&e, 0, 8, "") < 0, "only engine slots");

    // Played: lane a param 1 at 255, lane b "tone" at 0, both on tracks 1-5.
    Store store;
    Draft d = {};
    d.song = 0, d.entry = 1, d.offset = 0, d.length = 16, d.scene_before = kNoScene, d.scene_after = kNoScene;
    clear_lane(d.lanes[2]);  // build 060: lane C stays empty here (these lanes are the two the test is about)
    for (u32 n = 0; n < kLanesV10; ++n) {
        DraftLane& l = d.lanes[n];
        clear_lane(l);
        l.gesture = 0, l.selection = 0x1f, l.default_target = false, l.target = n;
        l.count = 1;
        l.points[0] = Point{0, static_cast<unsigned char>(n ? 0 : 255), 0};
    }
    engine::copy(d.lanes[1].name, "tone", kNameCapacity);
    require(store.put(0, d), "put");
    SongPosition s = {};
    s.playing = true, s.song = 0, s.entry = 1, s.next_entry = 0, s.position = 4 * kStepTicks, s.length = 64 * kStepTicks;
    s.scene = kNoScene, s.scene_before = kNoScene;
    Targets t;
    targets(store, s, &t, 0, &e);
    u32 v;
    require(t.find(0, 0, &v) && v == 255u << 8 && t.find(1, 0, &v) && t.find(3, 0, &v) && t.find(4, 0, &v), "param 1 on 1, 2, 4, 5");
    require(t.find(2, 0, &v), "and the drum's unnamed slot 1 (build 034)");
    require(t.find(1, 1, &v) && v == 0 && t.find(4, 1, &v) && v == 0, "tone on tracks 2 and 5 (slot 2)");
    require(t.find(0, 0, &v) && v == 255u << 8 && !t.find(0, 1, &v), "track 1's tone is slot 1: lane a's, met first");
    require(t.count == 7, "nothing else");
    targets(store, s, &t);  // the notes' check: no names read
    require(t.find(2, 0, &v) && !t.find(1, 1, &v), "without Engines: by slot alone, nothing by name");

    // Stored: version 6, the name after the curve (flags bit 3), read back.
    {
        const Variation& x = store[0];
        const u32 lane_b = x.start + kHeader + kLaneHeader + 3;  // lane a: 7 bytes and a point
        require(store.bytes()[2] == 12 && store.size() == 4 + 9 + 7 + 3 + 7 + 1 + 4 + 3 + 7, "version 12; lane b 5 bytes longer, lane C empty (build 060: 7 more)");
        require(store.bytes()[lane_b + 4] == (kLaneNamed) && store.bytes()[lane_b + 6] == 4 &&
                    !std::memcmp(store.bytes() + lane_b + 7, "tone", 4) && store.bytes()[lane_b + 11] == 1,
                "lane b: flags bit 3, the name's length and characters, then its points");
        char name[kNameCapacity];
        store.name(x, 1, name);
        require(!std::strcmp(name, "tone") && x.lanes[0].name_length == 0, "the name read back; lane a none");
        Draft back;
        store.draft(0, back);
        require(!std::strcmp(back.lanes[1].name, "tone") && !back.lanes[0].name[0], "and into a draft");
        static u8 copy[kCapacity];
        const u32 n = store.size();
        std::memcpy(copy, store.bytes(), n);
        Store other;
        require(other.load(copy, n) && other.count() == 1 && other[0].lanes[1].name_length == 4, "loads");
        // Damage: refused.
        for (u32 cut = 1; cut < n; ++cut) require(!other.load(copy, cut), "truncated: refused");
        copy[lane_b + 6] = 0;
        require(!other.load(copy, n), "a name of no characters");
        copy[lane_b + 6] = 4, copy[lane_b + 7] = 'T';
        require(!other.load(copy, n), "a capital: refused (names are lowercase)");
        copy[lane_b + 7] = 't', copy[lane_b + 2] = 16;
        require(!other.load(copy, n), "a name on a fixed target");
        copy[lane_b + 2] = 1;
        require(other.load(copy, n), "as it was");
        // A name only on an engine target, and at most 15 characters.
        Draft f = d;
        f.lanes[1].target = 16;
        require(store.put(0, f) && store.size() == n - 5 && store[0].lanes[1].name_length == 0, "a fixed target stores no name");
        f.lanes[1].target = 2;
        engine::copy(f.lanes[1].name, "a fifteen chars", kNameCapacity);
        require(store.put(0, f) && store[0].lanes[1].name_length == 15, "15 characters");
        engine::copy(f.lanes[1].name, "Tone", kNameCapacity);
        require(!store.put(0, f), "a capital is no name");
    }
    // Version 5 bytes: version 6's with no names; renumbered as they load. Flags bit 3 meant
    // nothing then: cleared, not read as a name.
    {
        static const u8 v5[] = {86, 82, 5, 1, 0, 1, 0, 0, 16, 0, 0, 255, 255, 0, 128, 0, 1, 2 | 8, 0, 1, 0, 255, 0,
                                0xff, 0, 0xff, 0, 2, 0, 0};
        Store old;
        require(old.load(v5, sizeof v5) && old.count() == 1 && old.version() == 12 && old.bytes()[2] == 12, "version 5: read as 12");
        // Build 060: the same bytes but for the empty lane C written in (7 more) and the number.
        require(old.size() == sizeof v5 + kLaneHeader && old[0].lanes[0].name_length == 0 && old.bytes()[4 + 9 + 4] == 2 &&
                    old[0].lanes[2].gesture == kNoGesture,
                "the same bytes, lane C empty; bit 3 cleared");
        require(old[0].lanes[0].target == 0 && old[0].lanes[0].count == 1, "an engine slot: played by slot");
    }
    std::puts("variations engine controls: ok");
    return 0;
}
