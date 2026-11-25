// Host test: the extension end to end on the SDK's doubles (scene clock and
// track values): examples/first-test.json loaded as saved state (as build 003
// saved it: version 1, rewritten as version 6), played through a song of two
// 4-bar entries; then a variation whose two lanes move different targets on
// different tracks, a release among them (stored mirrored).
#include "../extension.cpp"
#include <opxy/extension_testing.hpp>
#include <cstdio>
#include <cstdlib>
using namespace opxy::extensions;
namespace vx = opxy::extensions::variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
// examples/first-test.json, encoded (test_variations_tool.py keeps it current).
static const opxy::u8 saved[] = {86, 82, 1, 2, 0, 1, 224, 255, 32, 0, 0, 0, 3, 0, 0, 2, 16, 2, 0, 128, 0, 31, 0, 0, 33, 2, 0, 0, 0, 31, 255, 40, 0, 1, 32, 0, 32, 0, 0, 0, 12, 0, 0, 3, 50, 2, 0, 128, 0, 31, 0, 196, 3, 2, 0, 255, 0, 31, 0, 0, 19, 3, 0, 128, 0, 15, 255, 0, 31, 255, 0};
static testing::SceneClock clock_;
static testing::TrackValues values;
static opxy::u32 entry = 0;
static opxy::u32 pass_ticks = 20;
static Context now() {
    SongPosition s = {};
    s.song = 0;
    s.entry = entry;
    s.entries = 2;
    s.next_entry = entry == 0 ? 1 : 0;
    s.loops = true;
    Context c = {};
    c.recording.project = 1;
    c.playing = clock_.playing;
    c.song = clock_.after_pass(s);
    return c;
}
// One audio block's pass at 120 BPM: 480 a step, 16th = 125 ms, so about 5.6 a block; use 20.
static bool pass() {
    const bool ended = clock_.pass(pass_ticks);
    if (ended) entry ^= 1;
    values.pass(vx::hooks, now());
    return ended;
}
static opxy::u32 step() { return clock_.position / 480; }
int main() {
    const SavedState& st = vx::saved_state;
    st.load(saved, sizeof saved);
    opxy::u8 back[8192];
    const opxy::u32 upgraded = st.save(back, sizeof back);
    require(upgraded > sizeof saved && back[2] == 12 && back[3] == 3, "saves what it loaded, as version 12");
    // The song page's marks: both variations are pinned to song 1's join into
    // entry 2 (the second plays from it, 32 steps in); nothing for another song.
    {
        const opxy::u8 scenes[2] = {0, 1};
        SongMarks marks = {{0, 0, 0, 0}, {0, 0, 0, 0}};
        vx::song_marks(SongView{0, 2, scenes}, marks);
        require(marks.joins[0] == 0x2 && !marks.joins[1] && !marks.joins[2] && !marks.joins[3], "marks their join");
        SongMarks other = {{0, 0, 0, 0}, {0, 0, 0, 0}};
        vx::song_marks(SongView{1, 2, scenes}, other);
        require(!other.joins[0], "no marks on another song");
    }
    for (opxy::u32 t = 0; t < 8; ++t) {
        values.stock_sets(t, track_word::gain, 0x7fffffff);
        values.stock_sets(t, track_word::fx1_send, 0x19999999);
        values.stock_sets(t, track_word::cutoff, 0x5999999a);
        values.filter[t] = TrackFilter{16, true};
    }
    clock_.length = clock_.rule = 64 * 480;
    clock_.play();
    // Entry 1, bars 1-2: nothing.
    while (step() < 32) {
        pass();
        require(values.sends == 0, "entry 1, bars 1-2: as written");
    }
    // Bars 3-4: track 2's send swells towards full.
    opxy::i32 last = values.gui[1][track_word::fx1_send];
    while (!pass()) {
        if (clock_.position + 20 >= clock_.length) {
            // Values lead by one pass: the pass before the join already sends the join's,
            // the first of the ease back (build 016): a little below where the swell ended.
            require(values.dsp[1][track_word::fx1_send] < last && values.dsp[1][track_word::fx1_send] > 0x70000000,
                    "the ease back starts a pass early");
            last = values.dsp[1][track_word::fx1_send];
            continue;
        }
        require(values.dsp[1][track_word::fx1_send] >= last, "track 2's send rises");
        last = values.dsp[1][track_word::fx1_send];
    }
    require(last > 0x70000000, "nearly full by the join");
    // Build 074: then back to as set, going down every pass. The swell ended near full (a jump of
    // nearly the whole range), so the climb out takes nearly all of kEaseOutPasses (32 passes, 640
    // ticks at 20 a pass), well over the 6 of the ease in, and arrives exactly at the GUI's value.
    {
        opxy::u32 passes = 0;
        while (values.dsp[1][track_word::fx1_send] != values.gui[1][track_word::fx1_send] && passes < 40) {
            require(values.dsp[1][track_word::fx1_send] < last, "eases back after the swell");
            last = values.dsp[1][track_word::fx1_send];
            pass(), ++passes;
        }
        require(passes >= 28 && passes <= 32, "the swell's ease back takes nearly 32 passes (46 ms), not 6");
    }
    require(values.dsp[1][track_word::fx1_send] == values.gui[1][track_word::fx1_send], "back exactly at the GUI's value");
    // Entry 2, bars 1-2: nothing of ours.
    while (step() < 31) pass();
    require(values.dsp[2][track_word::cutoff] == values.gui[2][track_word::cutoff], "entry 2 bars 1-2: as set");
    // The window starts at step 32. One pass ahead: on the pass that reaches it, the
    // value for the next pass (20 ticks in) is already sent; build 014 sent its start, as set.
    while (clock_.position < 32 * 480) pass();
    require(clock_.position == 32 * 480 && values.dsp[2][track_word::cutoff] < values.gui[2][track_word::cutoff],
            "values lead by one pass");
    // Bars 3-4: track 3's cutoff closes, track 4 fades.
    pass();
    while (step() < 63) pass();
    require(values.dsp[2][track_word::cutoff] < 0x10000000, "track 3 nearly closed");
    require(values.dsp[3][track_word::gain] < 0x08000000, "track 4 nearly silent");
    // M3 off on track 3: its cutoff is given back at once.
    values.filter[2].on = false;
    pass();
    require(values.dsp[2][track_word::cutoff] == values.gui[2][track_word::cutoff], "M3 off: given back");
    // Stop: everything stays held.
    // Build 076 ("I'd expect it to hold whatever volume it was at, even if that was 0,
    // but reset when you hit play, or play the keyboard etc."). CHANGED from "stop gives every
    // word back" (builds 052/074: only the silent volume was kept): now every driven word stays
    // exactly as the last running pass left it, and a live note gives them all back.
    opxy::i32 held[8][3];
    for (opxy::u32 t = 0; t < 8; ++t)
        held[t][0] = values.dsp[t][track_word::gain], held[t][1] = values.dsp[t][track_word::fx1_send],
        held[t][2] = values.dsp[t][track_word::cutoff];
    clock_.stop();
    values.pass(vx::hooks, now());
    for (opxy::u32 k = 0; k < 50; ++k) values.pass(vx::hooks, now());
    for (opxy::u32 t = 0; t < 8; ++t)
        require(values.dsp[t][track_word::gain] == held[t][0] && values.dsp[t][track_word::fx1_send] == held[t][1] &&
                values.dsp[t][track_word::cutoff] == held[t][2], "stop holds every word where it was");
    require(values.dsp[3][track_word::gain] == 0 && values.gui[3][track_word::gain] != 0, "stop keeps track 4's silent volume held");
    vx::hooks.input_activity();
    values.pass(vx::hooks, now());
    for (opxy::u32 t = 0; t < 8; ++t)
        require(values.dsp[t][track_word::gain] == values.gui[t][track_word::gain] &&
                values.dsp[t][track_word::fx1_send] == values.gui[t][track_word::fx1_send] &&
                values.dsp[t][track_word::cutoff] == values.gui[t][track_word::cutoff], "a live note gives every word back");
    // Two lanes in one variation (fixtures/two-lanes.json), on the join into entry 2:
    // lane a drops track 1's volume near the end, lane b raises the release of tracks 5
    // and 6 (towards long: stored mirrored, a lower word): points as builds before 015 made a
    // rise (up from as set), kept as stored.
    {
        static const opxy::u8 two[] = {86, 82, 12, 1, 0, 1, 224, 255, 32, 0, 0, 255, 255, 3, 255, 15, 1, 2, 0, 3, 0, 128, 0, 15, 128, 0, 16, 0, 127, 4, 255, 11, 48, 0, 60, 2, 0, 128, 0, 31, 255, 60, 255, 128, 255, 0, 2, 0, 0};
        st.load(two, sizeof two);
        require(st.save(back, sizeof back) == sizeof two, "loads as it is");
        // Build 060: the same variation as version 10 saved it (two lanes) loads with an empty lane C and saves as version 11.
        static const opxy::u8 two10[] = {86, 82, 10, 1, 0, 1, 224, 255, 32, 0, 0, 255, 255, 3, 255, 15, 1, 2, 0, 3, 0, 128, 0, 15, 128, 0, 16, 0, 127, 4, 255, 11, 48, 0, 60, 2, 0, 128, 0, 31, 255, 60};
        st.load(two10, sizeof two10);
        opxy::u32 saved10 = st.save(back, sizeof back);
        require(saved10 == sizeof two && back[2] == 12, "version 10 loads, saves as version 12 with lane C written in");
        for (opxy::u32 i = 3; i < saved10; ++i) require(back[i] == two[i], "the same bytes as the version 12 fixture");
        st.load(two, sizeof two);
        const opxy::i32 release = 0x40000000;
        for (opxy::u32 t = 0; t < 8; ++t) values.stock_sets(t, 11, release);
        entry = 0;
        clock_.length = clock_.rule = 64 * 480;
        clock_.position = clock_.carry = 0;
        clock_.play();
        while (step() < 33) pass();
        require(values.dsp[0][track_word::gain] == 0x7fffffff && values.dsp[4][11] < release && values.dsp[5][11] < release,
                "both lanes heard: the release lower (longer) on 5 and 6");
        require(values.dsp[4][track_word::gain] == 0x7fffffff && values.dsp[0][11] == release, "each on its own tracks");
        opxy::i32 last = values.dsp[4][11];
        while (step() < 63) {
            pass();
            require(values.dsp[4][11] <= last, "the release keeps lengthening");
            last = values.dsp[4][11];
        }
        require(last < 0x08000000, "nearly the longest by the join");
        require(values.dsp[0][track_word::gain] == 0, "track 1 broken off");
        while (!pass()) {}
        // The drop comes back at once (sudden on purpose: no ease after break and
        // drop); the release eases back over 6 passes (build 016).
        require(values.dsp[0][track_word::gain] == 0x7fffffff, "the drop back at the join, at once");
        require(values.dsp[4][11] < release, "the release still easing back");
        // Build 074: the release's climb back is a big jump, so it takes longer than the 6 passes of the
        // ease in (up to kEaseOutPasses = 32, scaled by the jump), going up every pass and arriving
        // exactly at the GUI's value.
        opxy::u32 back_in = 0;
        opxy::i32 up = values.dsp[4][11];
        while (values.dsp[4][11] != release && back_in < 40) {
            pass(), ++back_in;
            require(values.dsp[4][11] >= up, "the release comes back without dipping");
            up = values.dsp[4][11];
        }
        require(back_in > 6 && back_in <= 32, "the release's climb back is longer than the ease in, within 32 passes");
        require(values.dsp[4][11] == release && values.dsp[5][11] == release, "the release back exactly at the GUI's value");
        clock_.stop();
        values.pass(vx::hooks, now());
    }
    // A build on the join into entry 2 (fixtures/build-on-join.json): track 1 comes in at
    // step 8, track 2 at step 24. Its values lead by one pass, across the join too, and
    // a note on a track the build holds silent doesn't sound at all.
    {
        static const opxy::u8 onjoin[] = {86, 82, 12, 1, 0, 1, 0, 0, 32, 0, 0, 255, 255, 1, 255, 15, 3, 2, 0, 3, 0, 0, 0, 7, 0, 0, 8, 128, 127, 255, 128, 255, 0, 2, 0, 0, 255, 128, 255, 0, 2, 0, 0};
        st.load(onjoin, sizeof onjoin);
        require(st.save(back, sizeof back) == sizeof onjoin, "the build loads");
        Actions actions = values.actions(Actions{});
        entry = 0;
        clock_.length = clock_.rule = 64 * 480;
        clock_.position = clock_.carry = 0;
        clock_.play();
        pass(), pass();  // the pace: 20 ticks a pass
        // Entry 1 as set, until the ease into the build's silence (build 016): over
        // the 6 passes (120 ticks) before the join, sent a pass ahead, going down
        // every pass.
        opxy::i32 eased = 0x7fffffff;
        opxy::u32 easing = 0;
        while (clock_.position + 20 < clock_.length) {
            pass();
            const opxy::i32 g = values.dsp[0][track_word::gain];
            if (clock_.position + 20 + 120 <= clock_.length) {
                require(g == 0x7fffffff, "entry 1: as set");
            } else {
                require(g < eased && values.dsp[1][track_word::gain] == g, "eases down into the build");
                eased = g, ++easing;
            }
        }
        require(easing == 6, "6 passes of ease");
        require(entry == 0 && values.dsp[0][track_word::gain] == 0 && values.dsp[1][track_word::gain] == 0,
                "the pass before the join already sends the build's silence");
        require(pass() && entry == 1, "the join");
        auto note = [&](opxy::u32 track) {
            SequencedNote n = {};
            n.track = track, n.length = 240, n.velocity = 100;
            return vx::hooks.sequenced_note(now(), n, actions);
        };
        require(!note(0) && !note(1), "the window's first notes: dropped, not heard for a moment");
        require(note(2), "a track the build doesn't hold: plays");
        while (step() < 8) pass();
        require(note(0) && !note(1), "track 1 in at step 8: its notes play; track 2's still don't");
        require(values.dsp[0][track_word::gain] == 0x7fffffff, "track 1 as set");
        while (step() < 24) pass();
        require(note(1), "track 2 in at step 24");
        clock_.stop();
        values.pass(vx::hooks, now());
    }
    // Build 025 (format version 6): engine controls by slot and by name on a lane of mixed
    // engines, through the SDK's double. Lane a: param 1 up (slot 1 on every lane track,
    // whatever its engine calls it); lane b: "tone" down (on each track whose engine has a
    // control of that name, wherever it is). Track 1: tone, env, shape, tremolo; track 2:
    // pitch, tone, ...; track 3: no engine controls; track 4: slots without names.
    {
        using namespace ::variations;
        values.names[0][0] = "tone", values.names[0][1] = "env", values.names[0][2] = "shape", values.names[0][3] = "tremolo";
        values.names[1][0] = "pitch", values.names[1][1] = "Tone", values.names[1][2] = "drive", values.names[1][3] = "air";
        values.unused[2] = 0xff;
        Draft d = {};
        d.song = 0, d.entry = 1, d.offset = 0, d.length = 32, d.scene_before = kNoScene, d.scene_after = kNoScene;
        clear_lane(d.lanes[2]);  // build 060: lane C stays empty (the two lanes are what this checks)
        for (u32 n = 0; n < kLanesV10; ++n) {
            DraftLane& l = d.lanes[n];
            clear_lane(l);
            l.gesture = 0, l.selection = 0x0f, l.default_target = false;
            l.target = n ? 1 : 0;  // lane b: "tone" as found on track 2 (slot 2): played by name
            l.count = 2;
            l.points[0] = Point{0, static_cast<unsigned char>(n ? 0 : 255), 0};
            l.points[1] = Point{31, static_cast<unsigned char>(n ? 0 : 255), 0};
        }
        engine::copy(d.lanes[1].name, "tone", kNameCapacity);
        static Store encoder;
        encoder.clear();
        require(encoder.put(0, d), "encodes");
        require(encoder.bytes()[2] == 12 && encoder.size() == 9 + 3 * 7 + 1 + 4 + 2 * 6 + 4, "version 12, the name 1 + 4 bytes, lane C empty");
        st.load(encoder.bytes(), encoder.size());
        require(st.save(back, sizeof back) == encoder.size(), "loads as it is");
        const opxy::i32 own = 0x40000000;
        for (opxy::u32 t = 0; t < 4; ++t)
            for (opxy::u32 w = 0; w < 4; ++w) values.stock_sets(t, w, own);
        entry = 0;
        clock_.length = clock_.rule = 64 * 480;
        clock_.position = clock_.carry = 0;
        clock_.play();
        while (!pass()) {}
        while (step() < 8) pass();
        require(entry == 1, "in the window");
        require(values.dsp[0][0] == 0x7fffffff && values.dsp[1][0] == 0x7fffffff, "param 1: slot 1 up on tracks 1 (tone) and 2 (pitch)");
        require(values.dsp[1][1] == 0 && values.dsp[0][1] == own, "tone down on track 2 (its slot 2); track 1's tone is lane a's slot 1");
        require(values.dsp[2][0] == own && values.dsp[2][1] == own, "track 3: no engine controls, left alone");
        require(values.dsp[3][0] == 0x7fffffff && values.dsp[3][1] == own,
                "track 4: its unnamed slot 1 moved by param 1 (build 034), no tone");
        for (opxy::u32 t = 0; t < 4; ++t)
            require(values.dsp[t][2] == own && values.dsp[t][3] == own, "slots 3 and 4 untouched");
        clock_.stop();
        values.pass(vx::hooks, now());
        // Build 076: CHANGED from "given back on stop": Stop holds the driven words; a
        // live note gives them back.
        require(values.dsp[0][0] == 0x7fffffff && values.dsp[1][1] == 0, "held on stop");
        vx::hooks.input_activity();
        values.pass(vx::hooks, now());
        require(values.dsp[0][0] == own && values.dsp[1][1] == own, "given back at the first live note");
    }
    // Build 046: gate on the volume of all 8 tracks, 100%, rate 1, on the join into entry 2, at
    // several window lengths and the device's pass (about 6 ticks): a beat on (full), a beat off
    // (silent, its notes dropped), through the whole window.
    {
        using namespace ::variations;
        pass_ticks = 6;
        static const opxy::u32 lens[] = {16, 32, 64, 128};
        for (opxy::u32 L : lens) {
            Draft d = {};
            d.song = 0, d.entry = 1, d.offset = -static_cast<opxy::i32>(L), d.length = L;
            d.scene_before = kNoScene, d.scene_after = kNoScene;
            for (u32 n = 0; n < kLanes; ++n) clear_lane(d.lanes[n]);
            DraftLane& l = d.lanes[0];
            l.gesture = 13, l.amount = 255, l.target = kVolume, l.selection = 0xff, l.rate = 1;
            generate(l, d.length);
            static Store gate;
            gate.clear();
            require(gate.put(0, d), "encodes the gate");
            st.load(gate.bytes(), gate.size());
            for (opxy::u32 t = 0; t < 8; ++t) values.stock_sets(t, track_word::gain, 0x7fffffff);
            entry = 0;
            clock_.length = clock_.rule = (L + 8) * 480;
            clock_.position = clock_.carry = 0;
            clock_.play();
            Actions act = values.actions(Actions{});
            for (opxy::u32 s = 0; s < L - 1; ++s) {
                while (step() < 8 + s) pass();
                const bool on = s / 4 % 2 == 0;
                for (opxy::u32 t = 0; t < 8; ++t)
                    require(values.dsp[t][track_word::gain] == (on ? 0x7fffffff : 0), "gate gates every track's volume");
                SequencedNote nn = {};
                nn.track = 3, nn.length = 240, nn.velocity = 100;
                require(vx::hooks.sequenced_note(now(), nn, act) == on, "gate's off beats drop their notes");
            }
            clock_.stop();
            values.pass(vx::hooks, now());
        }
        pass_ticks = 20;
    }
    // Build 076 (2026-11-25): Stop holds every driven word (build 052 kept only the
    // silent volume, with a one-second backstop; the backstop is gone: "hold whatever volume
    // it was at, even if that was 0"). Track 1's volume is held at 0 by lane a, track 3's cutoff
    // at full by lane b. Held until a live note, Play, or a project change.
    {
        using namespace ::variations;
        Draft d = {};
        d.song = 0, d.entry = 1, d.offset = 0, d.length = 32, d.scene_before = kNoScene, d.scene_after = kNoScene;
        clear_lane(d.lanes[2]);  // build 060: lane C stays empty
        for (u32 n = 0; n < kLanesV10; ++n) {
            DraftLane& l = d.lanes[n];
            clear_lane(l);
            l.gesture = 0, l.default_target = false;
            l.count = 2;
            l.target = n ? kCutoff : kVolume;
            l.selection = n ? 0x04 : 0x01;  // lane a: track 1's volume, held at 0; lane b: track 3's cutoff, full
            const unsigned char v = n ? 255 : 0;
            l.points[0] = Point{0, v, 0};
            l.points[1] = Point{31, v, 0};
        }
        static Store silence;
        silence.clear();
        require(silence.put(0, d), "encodes the silent hold");
        st.load(silence.bytes(), silence.size());
        for (opxy::u32 t = 0; t < 8; ++t) {
            values.stock_sets(t, track_word::gain, 0x7fffffff);
            values.stock_sets(t, track_word::cutoff, 0x5999999a);
            values.filter[t] = TrackFilter{16, true};
        }
        auto play_to_window = [&] {
            entry = 0;
            clock_.length = clock_.rule = 64 * 480;
            clock_.position = clock_.carry = 0;
            clock_.play();
            while (!pass()) {}
            while (step() < 8) pass();
            require(entry == 1 && values.dsp[0][track_word::gain] < 0x01000000, "the lane holds track 1 silent");
            require(values.dsp[2][track_word::cutoff] != values.gui[2][track_word::cutoff], "and track 3's cutoff driven");
        };
        auto stop = [&] {
            clock_.stop();
            values.pass(vx::hooks, now());
        };
        const opxy::i32 gain = values.gui[0][track_word::gain];
        const opxy::i32 cutoff = values.gui[2][track_word::cutoff];
        // Stop: both words stay held, whatever they are.
        play_to_window();
        const opxy::i32 driven = values.dsp[2][track_word::cutoff];
        stop();
        require(values.dsp[0][track_word::gain] < 0x01000000, "stop: the silent volume stays held");
        require(values.dsp[2][track_word::cutoff] == driven, "stop: the cutoff stays where the lane left it");
        // Held far past the old one-second backstop (689 passes): no backstop now.
        for (opxy::u32 k = 0; k < 3 * 689; ++k) values.pass(vx::hooks, now());
        require(values.dsp[0][track_word::gain] < 0x01000000 && values.dsp[2][track_word::cutoff] == driven, "no backstop: still held");
        // A live note: flagged only by the hook; the next pass gives both back.
        vx::hooks.input_activity();
        require(values.dsp[0][track_word::gain] < 0x01000000, "input_activity only flags: nothing is sent from the hook");
        values.pass(vx::hooks, now());
        require(values.dsp[0][track_word::gain] == gain && values.dsp[2][track_word::cutoff] == cutoff, "a live note: the next pass gives both back");
        values.pass(vx::hooks, now());
        require(values.dsp[0][track_word::gain] == gain, "and they stay given back");
        // A second note after the restore does nothing more.
        vx::hooks.input_activity();
        values.pass(vx::hooks, now());
        require(values.dsp[0][track_word::gain] == gain && values.dsp[2][track_word::cutoff] == cutoff, "a later note changes nothing");
        // A note played while running, before the Stop, doesn't end the hold; nor does one
        // struck after the last running pass but before the first stopped pass.
        play_to_window();
        vx::hooks.input_activity();
        pass();
        stop();
        require(values.dsp[0][track_word::gain] < 0x01000000, "a note played while running leaves the Stop's hold alone");
        play_to_window();
        vx::hooks.input_activity();  // between the last running pass and the Stop
        stop();
        require(values.dsp[0][track_word::gain] < 0x01000000 && values.dsp[2][track_word::cutoff] != cutoff, "a note just before the Stop's first pass doesn't release it");
        // Play again during the hold: the running branch takes over (the restore on Play is
        // the first running pass's own targets).
        play_to_window();
        require(values.dsp[0][track_word::gain] < 0x01000000, "Play after a hold: the lane's values again");
        // Play after a hold at a place the lane doesn't reach: everything is given back by
        // the running pass's own targets (no variation there).
        stop();
        entry = 0;
        clock_.position = clock_.carry = 0;
        clock_.play();
        pass(), pass();
        require(entry == 0 && values.dsp[0][track_word::gain] == gain && values.dsp[2][track_word::cutoff] == cutoff,
                "Play where nothing is driven: the held words are back at the GUI's values");
        // A project change gives everything back at once, held or not.
        play_to_window();
        stop();
        require(values.dsp[0][track_word::gain] < 0x01000000, "held again before the project change");
        {
            Context c = now();
            c.recording.project = 2;
            values.project = 2;
            values.pass(vx::hooks, c);
            require(values.dsp[0][track_word::gain] == gain && values.dsp[2][track_word::cutoff] == cutoff, "a project change gives it all back");
            values.project = 1;
            values.pass(vx::hooks, now());  // back to project 1: nothing held
            require(values.dsp[0][track_word::gain] == gain, "and nothing is held again");
        }
    }
    // A project with none saves nothing.
    st.load(saved, 0);
    require(st.save(back, sizeof back) == 0, "none: saves 0");
    std::puts("variations extension: ok");
    return 0;
}
