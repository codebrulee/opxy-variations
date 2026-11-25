// Host test: a variation at the song's start plays from the moment playback
// starts there (build 014): on Play from the top, as the song loops round
// (also after the song's end was edited and not yet re-pinned), on Play
// straight into a later entry, and in a song that doesn't loop; the scene
// guard still silences a join whose scene before has changed. End to end
// through the extension's sequencer pass, on the SDK's scene clock and track
// values doubles. Each variation is a build on track 1's volume with amount 0:
// silent for its window's first half, then as set.
#include "../extension.cpp"
#include <opxy/extension_testing.hpp>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
using namespace opxy::extensions;
namespace vx = opxy::extensions::variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static const opxy::u32 kSteps = 64;  // every entry 4 bars
static opxy::u32 scenes[8];
static opxy::u32 entries = 0, entry = 0;
static bool loops = true;
static testing::SceneClock clock_;
static testing::TrackValues values;
static Context now() {
    SongPosition s = {};
    s.song = 0, s.entry = entry, s.entries = entries, s.next_entry = (entry + 1) % entries, s.loops = loops;
    s.scene = scenes[entry];
    Context c = {};
    c.recording.project = 1;
    c.playing = clock_.playing;
    c.song = clock_.after_pass(s);
    return c;
}
// One pass; at the end of a song that doesn't loop the transport stops.
static void pass() {
    if (clock_.pass(20)) {
        if (!loops && entry + 1 == entries) clock_.stop(), entry = 0;
        else entry = (entry + 1) % entries;
    }
    values.pass(vx::hooks, now());
}
static opxy::u32 step() { return clock_.position / 480; }
// Plays on to `at` steps into entry `e` (the next time it comes round).
static void play_to(opxy::u32 e, opxy::u32 at) {
    for (opxy::u32 guard = 0; guard < 100000; ++guard) {
        if (clock_.playing && entry == e && step() >= at) return;
        pass();
    }
    require(false, "never got there");
}
static void play_from(opxy::u32 e) {
    clock_.stop();
    values.pass(vx::hooks, now());  // a stopped pass: everything given back
    entry = e;
    clock_.length = clock_.rule = kSteps * 480;
    clock_.position = clock_.carry = 0;
    clock_.play();
    pass();
}
static opxy::i32 gain() { return values.dsp[0][track_word::gain]; }
static const opxy::i32 kFull = 0x7fffffff;
// A build on track 1's volume at the join into `at`, `offset` steps from it, 32 long.
static void add(opxy::u32 at, opxy::i32 offset, opxy::u32 before, opxy::u32 after) {
    ::variations::Draft d = {};
    d.song = 0, d.entry = at, d.offset = offset, d.length = 32;
    d.scene_before = before, d.scene_after = after;
    ::variations::clear_lane(d.lanes[0]), ::variations::clear_lane(d.lanes[1]), ::variations::clear_lane(d.lanes[2]);
    d.lanes[0].gesture = ::variations::build, d.lanes[0].amount = 0, d.lanes[0].selection = 1;
    d.lanes[0].target = ::variations::kVolume;
    ::variations::generate(d);
    require(vx::store.put(vx::store.count(), d), "put");
}
static void song(std::initializer_list<opxy::u32> s, bool loop = true) {
    entries = 0;
    for (opxy::u32 sc : s) scenes[entries++] = sc;
    loops = loop;
}
int main() {
    for (opxy::u32 t = 0; t < 8; ++t) values.stock_sets(t, track_word::gain, kFull);
    // Scenes 1, 2, 3; a fade-in on the song's start, on its join (into entry 1).
    song({0, 1, 2});
    vx::store.clear();
    add(0, 0, 2, 0);
    play_from(0);
    require(gain() == 0, "Play from the top: silent from the first pass");
    play_to(0, 12);
    require(gain() == 0, "still building at step 12");
    play_to(0, 17);
    require(gain() == kFull, "in at step 16");
    play_to(2, 60);
    require(gain() == kFull, "as set through the song");
    play_to(0, 1);
    require(gain() == 0, "the song loops round: the fade-in again");
    play_to(0, 17);
    require(gain() == kFull, "and in again");
    // The song's end edited (scene 3 > scene 4), not yet re-pinned (the page
    // hasn't opened): the start still plays, on Play and on the loop.
    song({0, 1, 3});
    play_from(0);
    require(gain() == 0, "an edited end: Play from the top plays");
    play_to(0, 17);
    play_to(2, 60);
    play_to(0, 1);
    require(gain() == 0, "an edited end: the loop round plays");
    // The guard still holds mid-song: a join whose scene before is no longer
    // the one it was made on (scene 4 > scene 2 now 1 > 2) stays silent when
    // the song arrives from that other scene.
    vx::store.clear();
    add(1, 0, 3, 1);
    play_from(0);
    play_to(1, 2);
    require(gain() == kFull, "arrived from another scene: silent");
    // Play straight into a later entry (the song page's cursor there): playback
    // starts at this entry, so its window plays from the first pass.
    play_from(1);
    require(gain() == 0, "Play into entry 2: plays from the first pass");
    play_to(1, 17);
    require(gain() == kFull, "and in at step 16");
    // A join that is right plays when the song steps into it.
    vx::store.clear();
    add(1, 0, 0, 1);
    play_from(0);
    play_to(1, 2);
    require(gain() == 0, "arrived from its scene before: plays");
    // A song that doesn't loop: its start plays on Play; nothing comes before
    // it, so a window ending on the start doesn't play at the song's end.
    song({0, 1, 2}, false);
    vx::store.clear();
    add(0, 0, 2, 0);   // the fade-in, on the start
    add(0, -16, 2, 0); // a window over the start: its first half before it
    play_from(0);
    require(gain() == 0, "no loop: Play from the top plays");
    play_to(0, 17);
    require(gain() == kFull, "in at step 16");
    play_to(2, 50);
    require(gain() == kFull, "no loop: nothing before the song's end (no join follows)");
    for (opxy::u32 guard = 0; clock_.playing && guard < 100000; ++guard) pass();
    require(!clock_.playing && gain() == kFull, "the song stops; everything given back");
    play_from(0);
    require(gain() == 0, "Play again: the start plays again");
    std::puts("variations song start: ok");
    return 0;
}
