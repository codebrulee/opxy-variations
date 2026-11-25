// Host test: mixer.hpp sends each lane's word, with the knob's direction (a
// release is stored mirrored), gives it back, respects M3 and the SDK's 32
// sends a pass.
#include "../mixer.hpp"
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
using namespace variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static i32 gui[kTracks][64], dsp[kTracks][64];
static u32 sends, ftype[kTracks], budget = 1000;
static bool fon[kTracks], loading;
static bool io_read(u32 t, u32 w, i32* v) { *v = gui[t][w]; return !loading; }
static bool io_filter(u32 t, u32* type, bool* on) { *type = ftype[t]; *on = fon[t]; return true; }
static bool io_send(u32 t, u32 w, i32 v) {
    if (loading || !budget) return false;
    --budget;
    dsp[t][w] = v;
    ++sends;
    return true;
}
static u32 calls;  // send calls of a pass, taken or refused (the SDK's 32 a callback count the calls)
static bool io_send_counted(u32 t, u32 w, i32 v) {
    ++calls;
    return io_send(t, w, v);
}
static const TrackIo io = {io_read, io_filter, io_send};
static Mixer mixer;
static Targets tg;
static void drive(u32 track, u32 word, u32 value) {
    for (u32 i = 0; i < tg.count; ++i)
        if (tg.drive[i].track == track && tg.drive[i].word == word) {
            tg.drive[i].value = value;
            return;
        }
    tg.add(track, word, value);
}
static void stop(u32 track, u32 word) {
    for (u32 i = 0; i < tg.count; ++i)
        if (tg.drive[i].track == track && tg.drive[i].word == word) {
            tg.drive[i] = tg.drive[--tg.count];
            tg.asked[track][word >> 5] &= ~(1u << (word & 31));
        }
}
static const u32 kTop = 255 << 8, kBottom = 0;
int main() {
    require(toward(0, kFull, 0xff00) == kFull && toward(kFull, 0, 0xff00) == 0, "toward: ends");
    require(toward(0, kFull, 0) == 0, "toward: start");
    const i32 half = toward(0, kFull, 0x8000);
    require(half > 0x40000000 && half < 0x40900000, "toward: about half");
    const i32 release = 0x40000000;  // the knob about half way
    for (u32 t = 0; t < kTracks; ++t) {
        gui[t][kVolume] = 0x5c28f5c2;    // engine volume 72 %
        gui[t][kSend] = 0x19999999;      // send 0.2
        gui[t][kCutoff] = 0x5999999a;    // cutoff 0.7
        gui[t][11] = release;
        gui[t][24] = 0x30000000;         // lfo speed
        fon[t] = true;
        ftype[t] = 16;                   // ladder
    }
    // The middle is as set: nothing sent.
    drive(0, kVolume, kMiddle);
    mixer.apply(tg, io);
    require(sends == 0 && !mixer.ours(0, kVolume), "as set: stock's value stands");
    // Volume to silence, send to full, cutoff closed, on track 1.
    drive(0, kVolume, kBottom), drive(0, kSend, kTop), drive(0, kCutoff, kBottom);
    mixer.apply(tg, io);
    require(dsp[0][kVolume] == 0 && dsp[0][kSend] == kFull && dsp[0][kCutoff] == kClosed && sends == 3, "the three words");
    mixer.apply(tg, io);
    require(sends == 3, "unchanged: not sent again");
    // Volume above as set: towards full (the knob clockwise).
    drive(0, kVolume, kTop);
    mixer.apply(tg, io);
    require(dsp[0][kVolume] == kFull, "volume up: full");
    // Version 2's level values (0..255, 255 as set) as rewritten (0..128): the same gain.
    drive(0, kVolume, legacy::value(legacy::level, 128) << 8);
    mixer.apply(tg, io);
    require(dsp[0][kVolume] > gui[0][kVolume] / 2 - 0x00800000 && dsp[0][kVolume] < gui[0][kVolume] / 2 + 0x00800000,
            "a version 2 level of 128: half the gain");
    // The cutoff's middle is the GUI's; above it opens to kOpen.
    drive(0, kCutoff, kMiddle);
    mixer.apply(tg, io);
    require(dsp[0][kCutoff] == gui[0][kCutoff], "the middle: as set");
    drive(0, kCutoff, kTop);
    mixer.apply(tg, io);
    require(dsp[0][kCutoff] == kOpen, "top: open");
    // M3 off or dynamic: the filter's words are given back and left alone.
    fon[0] = false;
    mixer.apply(tg, io);
    require(dsp[0][kCutoff] == gui[0][kCutoff] && !mixer.ours(0, kCutoff), "M3 off: back to the GUI's");
    fon[0] = true;
    ftype[0] = 18;
    u32 before = sends;
    mixer.apply(tg, io);
    require(sends == before, "dynamic: untouched");
    ftype[0] = 16;
    // A word no longer driven goes back.
    stop(0, kSend);
    mixer.apply(tg, io);
    require(dsp[0][kSend] == gui[0][kSend] && !mixer.ours(0, kSend), "a lane that stops gives its word back");

    // Direction: the release is stored mirrored (a higher word is shorter), so
    // a lane above as set (the knob clockwise: longer) lowers the word.
    drive(1, 11, kTop);
    mixer.apply(tg, io);
    require(dsp[1][11] == 0, "release up: the longest, word 0");
    drive(1, 11, 192 << 8);
    mixer.apply(tg, io);
    require(dsp[1][11] < release && dsp[1][11] > release / 4, "release part way up: a lower word");
    drive(1, 11, kBottom);
    mixer.apply(tg, io);
    require(dsp[1][11] == kFull, "release down: the shortest, the highest word");
    drive(1, 11, 64 << 8);
    mixer.apply(tg, io);
    require(dsp[1][11] > release && dsp[1][11] < kFull, "release part way down: a higher word");
    require(direction(11) == -1 && direction(35) == -1 && direction(8) == 1 && direction(kCutoff) == 1, "the SDK's directions");
    // The other way round for an ordinary word (the LFO speed): up is a higher word.
    drive(1, 24, kTop);
    mixer.apply(tg, io);
    require(dsp[1][24] == kFull, "lfo speed up: full");
    // A word already at its end stays there.
    gui[2][11] = 0;
    drive(2, 11, kTop);
    before = sends;
    mixer.apply(tg, io);
    require(sends == before && !mixer.ours(2, 11), "already the longest: nothing to send");

    // Two lanes on different tracks and targets: each word its own.
    tg.clear();
    mixer.apply(tg, io);
    drive(3, kVolume, kBottom);  // lane a: a break on track 4
    drive(5, kCutoff, kTop);     // lane b: a rise on track 6's cutoff
    mixer.apply(tg, io);
    require(dsp[3][kVolume] == 0 && dsp[5][kCutoff] == kOpen, "both lanes heard");
    require(!mixer.ours(5, kVolume) && !mixer.ours(3, kCutoff) && !mixer.ours(5, kSend), "each on its own track only");
    // The knob moves under a volume line: the line rides it.
    drive(3, kVolume, 0x4000);
    mixer.apply(tg, io);
    gui[3][kVolume] = 0x40000000;
    mixer.apply(tg, io);
    require(dsp[3][kVolume] > 0x1f000000 && dsp[3][kVolume] < 0x21000000, "half of the new volume");
    // While a project loads nothing goes; then it does.
    loading = true;
    drive(3, kVolume, kBottom);
    mixer.apply(tg, io);
    require(dsp[3][kVolume] != 0, "loading: nothing sent");
    loading = false;
    mixer.apply(tg, io);
    require(dsp[3][kVolume] == 0, "then sent");
    // Release: everything back to the GUI's.
    mixer.release(io);
    for (u32 t = 0; t < kTracks; ++t)
        for (u32 w : {kVolume, kCutoff, kSend, 11u, 24u}) require(!mixer.ours(t, w), "release: none ours");
    require(dsp[3][kVolume] == gui[3][kVolume] && dsp[5][kCutoff] == gui[5][kCutoff] && dsp[1][11] == gui[1][11] &&
            dsp[1][24] == gui[1][24] && dsp[0][kVolume] == gui[0][kVolume], "every word sent given back");
    const u32 after = sends;
    mixer.release(io);
    require(sends == after, "release twice: nothing more");

    // Budget: the SDK sends 32 a callback. 32 words driven at once go in one
    // pass; giving them back while others start takes a second pass, nothing
    // lost. The asks themselves are not bounded by that budget (kMaxDrives).
    tg.clear();
    for (u32 t = 0; t < kTracks; ++t)
        for (u32 w : {kVolume, kCutoff, kSend, 24u}) drive(t, w, kTop);
    require(tg.count == 32, "32 words");
    budget = 32;
    mixer.apply(tg, io);
    require(!budget && mixer.ours(7, 24), "all 32 in one pass");
    tg.clear();
    for (u32 t = 0; t < kTracks; ++t) drive(t, 11, kTop);  // 8 new words, 32 to give back
    budget = 32;
    mixer.apply(tg, io);
    budget = 32;
    mixer.apply(tg, io);
    for (u32 t = 0; t < kTracks; ++t) {
        require(dsp[t][11] == 0, "the new words, within two passes");
        for (u32 w : {kVolume, kCutoff, kSend, 24u}) require(dsp[t][w] == gui[t][w] && !mixer.ours(t, w), "the old given back");
    }
    // Build 024: an engine control on a lane of mixed engines: each track moves its own
    // engine's control in that slot; a track whose engine has none there is left alone.
    {
        struct Has { static bool has(u32 t, u32 w) { return !(t == 0 && w < 4); } };  // track 1: a drum engine, no slots
        const TrackIo mixed = {io_read, io_filter, io_send, Has::has};
        tg.clear();
        mixer.apply(tg, io), mixer.apply(tg, io);  // everything given back first
        for (u32 t = 0; t < 3; ++t) gui[t][0] = dsp[t][0] = 0x40000000;
        tg.clear();
        drive(0, 0, kTop), drive(2, 0, kTop);
        budget = 32;
        mixer.apply(tg, mixed);
        require(dsp[0][0] == 0x40000000 && !mixer.ours(0, 0), "track 1, no such slot: left alone");
        require(dsp[2][0] == kFull && mixer.ours(2, 0), "track 3's own first control moved");
        tg.clear();
        mixer.apply(tg, mixed);
        require(dsp[2][0] == 0x40000000, "and given back");
    }
    // Build 060 (version 11): three lanes ask for up to 3 x 8 = 24 words in a pass. Several variations overlapping
    // ask for more, and the list holds every distinct ask (window.hpp kMaxDrives is every ask there is, so it cannot
    // fill); what the SDK's 32 sends a callback bounds is the sending, and a word it refuses goes on the next pass.
    // Asserted on the real lanes through the real mixer: every send call of the pass is counted, and none is past 32.
    {
        tg.clear();
        mixer.apply(tg, io), mixer.apply(tg, io);  // everything given back first
        for (u32 t = 0; t < kTracks; ++t)
            for (u32 w : {kVolume, kCutoff, kSend, 8u, 9u, 10u, 11u, 24u, 25u}) gui[t][w] = dsp[t][w] = 0x40000000;
        auto lanes = [](Draft& d, u32 first, u32 a, u32 b, u32 c) {
            d.song = 0, d.entry = 1, d.offset = 0, d.length = 32, d.scene_before = d.scene_after = kNoScene;
            const u32 words[kLanes] = {a, b, c};
            for (u32 n = 0; n < kLanes; ++n) {
                DraftLane& l = d.lanes[n];
                clear_lane(l);
                l.gesture = 0, l.amount = 128, l.selection = 0xff, l.target = words[n], l.default_target = false;
                l.count = 2, l.points[0] = Point{0, static_cast<unsigned char>(first), 0}, l.points[1] = Point{31, static_cast<unsigned char>(first), 0};
            }
        };
        static Store store;
        Draft d = {};
        lanes(d, 255, kVolume, kCutoff, kSend);
        require(store.put(0, d), "three lanes on all eight tracks");
        const SongPosition pos = {true, 0, 1, 2, 8 * 480, 64 * 480, 0, kNoScene};
        targets(store, pos, &tg);
        require(tg.count == kLanes * kTracks, "24 words wanted");
        calls = 0;
        const TrackIo counted = {io_read, io_filter, io_send_counted};
        budget = 32;  // the SDK's 32 a callback
        const u32 before = sends;
        mixer.apply(tg, counted);
        require(calls == 24 && sends - before == 24 && budget == 8, "three lanes: 24 sends in the pass, within 32");
        // A second variation on the same window with other words (volume stays taken: the first lane to ask holds it):
        // the words wanted stop at kMaxDrives however many lanes ask.
        Draft e = {};
        lanes(e, 0, 24, 25, 11);
        require(store.put(1, e), "a second variation of three lanes");
        Draft f = {};
        lanes(f, 0, 8, 9, 10);
        store.put(2, f);
        targets(store, pos, &tg);
        require(tg.count == 9 * kTracks, "three variations: all 72 words asked, none dropped");
        calls = 0, budget = 32;
        const u32 before_all = sends;
        mixer.apply(tg, counted);
        // 33 calls, not 72: the refused one stops the rest being asked for.
        require(calls == 33 && sends - before_all == 32, "the pass sends 32 and stops at the refusal");
        // What the budget refused is not lost: the next passes send the rest.
        for (u32 pass = 0; pass < 3; ++pass) calls = 0, budget = 32, mixer.apply(tg, counted);
        for (u32 t = 0; t < kTracks; ++t)
            for (u32 w : {kVolume, kCutoff, kSend, 8u, 9u, 10u, 11u, 24u, 25u})
                require(mixer.ours(t, w), "every word asked reaches the board within a few passes");
    }
    std::puts("variations mixer: ok");
    return 0;
}
