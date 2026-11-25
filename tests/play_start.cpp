// Host test (build 018): a fade in on the song's start makes no sound at Play
// and eases in on the loop; a fade out into a join eases back after it. End to
// end through the extension's sequencer pass and sequenced_note, on the SDK's
// scene clock and track values doubles.
//
// - While stopped (sequencer_pass runs then too) every word is stock's own:
//   nothing of ours is sent, and anything held is given back at Stop, so the
//   keys sound as set (build 017 sent the values where Play starts, holding a
//   fade in's tracks silent on the keys). Counting in, the transport plays:
//   the values where Play started are in place.
// - Play's first notes, which still see the stopped position, are judged where
//   Play starts: the entry the SDK gives stopped as Play's own (the song's
//   first entry unless the song page moved it: stock's Play puts it back to 0
//   first). A note on a track the fade holds silent is dropped, and the first
//   pass running sends the silence, so no note sounds at full volume at Play.
// - Should stock send its own values as the transport starts, ours go again on
//   the first pass running; and whenever stock's own value changes under ours.
// - A few passes before a window that starts away from as set, the value eases
//   into its start value, arriving as the window starts; after a window that
//   ends far from as set, it eases back. Never a jump.
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
static const opxy::u32 kSteps = 64;  // every entry 4 bars
static const opxy::u32 kTicks = 20;  // a pass
static const opxy::u32 scenes[3] = {0, 1, 2};
static const opxy::u32 entries = 3;
static opxy::u32 entry = 0, next_entry = 1, song_index = 0, queued = no_scene;
static testing::SceneClock clock_;
static testing::TrackValues values;
static bool counting_in = false;
static Context now() {
    SongPosition s = {};
    s.song = song_index;
    if (song_index != no_song) s.entry = entry, s.entries = entries, s.next_entry = next_entry, s.loops = true;
    s.scene = scenes[entry];
    s.queued = queued;
    Context c = {};
    c.recording.project = 1;
    c.playing = clock_.playing || counting_in;
    c.song = clock_.after_pass(s);
    return c;
}
static void pass() {
    if (clock_.pass(kTicks)) entry = next_entry;
    if (clock_.playing) next_entry = (entry + 1) % entries;
    values.pass(vx::hooks, now());
}
static opxy::i32 gain(opxy::u32 t = 0) { return values.dsp[t][track_word::gain]; }
static const opxy::i32 kFull = 0x7fffffff;
static Actions actions;
static bool note(const Context& c, opxy::u32 track) {
    SequencedNote n = {};
    n.track = track, n.length = 240, n.velocity = 100;
    return vx::hooks.sequenced_note(c, n, actions);
}
static void add(opxy::u32 at, opxy::i32 offset, opxy::u32 length, opxy::u32 gesture, opxy::u32 amount, opxy::u32 tracks) {
    ::variations::Draft d = {};
    d.song = 0, d.entry = at, d.offset = offset, d.length = length;
    d.scene_before = scenes[(at + entries - 1) % entries], d.scene_after = scenes[at];
    ::variations::clear_lane(d.lanes[0]), ::variations::clear_lane(d.lanes[1]), ::variations::clear_lane(d.lanes[2]);
    ::variations::DraftLane& l = d.lanes[0];
    l.gesture = gesture, l.amount = amount, l.selection = tracks;
    l.curve = ::variations::default_curve(gesture);
    l.target = ::variations::natural_target(gesture);
    ::variations::generate(d);
    require(vx::store.put(vx::store.count(), d), "put");
}
static void stop() {
    clock_.stop();
    pass();
}
int main() {
    actions = values.actions(Actions{});
    for (opxy::u32 t = 0; t < 8; ++t) values.stock_sets(t, track_word::gain, kFull);
    clock_.length = clock_.rule = kSteps * 480;
    // A fade in from silence on tracks 1 and 2, on the song's start (2 bars).
    vx::store.clear();
    add(0, 0, 32, ::variations::fade_in, 255, 0x03);

    // Stopped with the song page's next entry moved to entry 3: Play starts
    // there, so its first notes play.
    entry = 1, next_entry = 2;
    pass();
    require(gain() == kFull, "stopped, Play starts entry 3: nothing of ours");
    clock_.play();
    {
        Context first = now();
        first.song.running = false, first.song.position = first.song.length = 0;
        require(note(first, 0), "Play at entry 3: its first note plays");
    }
    clock_.stop();
    // Stopped as usual: Play starts the song's first entry (the SDK says so).
    // Nothing is sent while stopped: the fade's tracks sound as set on the keys.
    next_entry = 0;
    for (int i = 0; i < 4; ++i) pass();
    require(gain() == kFull && gain(1) == kFull && gain(2) == kFull, "stopped: nothing of ours, the keys sound as set");
    require(!vx::mixer.ours(0, track_word::gain) && !vx::mixer.ours(1, track_word::gain), "stopped: no word held");
    song_index = 1;
    pass();
    song_index = no_song;
    pass();
    song_index = 0, queued = 1;
    pass();
    queued = no_scene;
    pass();
    require(gain() == kFull && gain(1) == kFull, "stopped, whatever the song: still nothing of ours");

    // Play. Its first pass's notes start before the clock reads as running:
    // the context they see is the stopped one, so they are judged at the start.
    clock_.play();
    {
        Context first = now();
        first.song.running = false, first.song.position = first.song.length = 0;
        require(first.playing && !note(first, 0) && !note(first, 1), "Play's first notes on the fade's tracks: dropped");
        require(note(first, 2), "another track's first note plays");
    }
    entry = 0, next_entry = 1;  // the boundary step Play runs: entry 1 starts
    // The volume is still stock's as the first pass running starts (nothing
    // was sent stopped); were stock to send its own values as the transport
    // starts, it would be the same. That pass sends the fade's silence.
    require(gain() == kFull && gain(1) == kFull, "at Play: stock's volume, from stopped");
    pass();
    require(gain() <= kFull / 255 * 2 && gain(1) <= kFull / 255 * 2, "the first running pass: silence sent");
    {
        Context c = now();
        require(!note(c, 0) && !note(c, 1), "the first running pass's notes on the fade's tracks: dropped");
    }
    // Stock's own value changing under ours (a p-lock, another pattern's
    // volume): stock sends it, and ours goes again on the next pass.
    values.stock_sets(0, track_word::gain, kFull / 2);
    pass();
    require(gain() < kFull / 64, "stock's change under the fade: silent again");
    values.stock_sets(0, track_word::gain, kFull);
    pass();
    require(gain() < kFull / 64, "and back: silent again");
    {
        // The fade comes up through its window, never a jump down.
        opxy::i32 last = gain();
        while (clock_.position < 32 * 480) {
            pass();
            require(gain() >= last, "the fade rises");
            last = gain();
        }
        require(gain() == kFull, "as set by the window's end");
    }

    // On the loop: entry 3's last passes ease down into the fade's silence,
    // arriving as the song comes round, a sixth or so a pass, never a jump.
    while (!(entry == 2 && clock_.position + kTicks * 8 >= clock_.length)) pass();
    require(gain() == kFull, "entry 3: as set before the ease");
    {
        opxy::i32 last = gain();
        opxy::u32 steps = 0;
        while (entry == 2) {
            pass();
            if (entry != 2) break;
            if (gain() != last) {
                require(gain() < last && last - gain() <= kFull / 6 + 0x01000000, "eases down in small steps");
                ++steps;
            }
            last = gain();
        }
        require(steps == 6 && last == 0, "6 passes of ease, silent (sent a pass ahead) as the song comes round");
        require(gain() == 0 && entry == 0, "the fade again from silence");
        Context c = now();
        require(!note(c, 0), "the loop's first note on the fade's track: dropped");
    }
    // Stop in the fade's silence: every word but the silent volumes back to
    // stock's at once (build 052, below); Play's first notes are still dropped.
    pass();
    require(gain() < kFull / 64, "in the fade's silence");
    stop();
    // Build 052 (2026-11-24): the track the lane holds silent stays held through
    // Stop (stock's own Stop releases its voices, and their tails must not ring out at
    // full level); the first live note, or the backstop, gives it back so the keys
    // sound as set.
    require(gain() < kFull / 64 && gain(1) < kFull / 64 && gain(2) == kFull, "Stop: the silent tracks stay held, the rest given back");
    vx::hooks.input_activity();
    values.pass(vx::hooks, now());
    require(gain() == kFull && gain(1) == kFull, "a live note: given back, the keys sound as set");
    next_entry = 0;  // stopped, the SDK gives Play's entry: the song's first
    pass();
    require(gain() == kFull, "stopped: still stock's");
    {
        clock_.play();
        Context first = now();
        first.song.running = false, first.song.position = first.song.length = 0;
        require(!note(first, 0), "Play again: its first note on the fade's track dropped");
        clock_.stop();
        pass();
    }

    // A count-in: the transport plays but the clock doesn't run yet; Play has
    // taken entry 1 already (the entry playing), and the next is entry 2.
    clock_.position = clock_.carry = 0;
    entry = 0, next_entry = 1, counting_in = true;
    pass();
    require(gain() == 0, "counting in: the start's values in place");
    counting_in = false;

    // Nothing in the project: everything goes back.
    vx::store.clear();
    pass();
    require(gain() == kFull && gain(1) == kFull, "no variations: given back");

    // A fade out to silence into the join into entry 2: after the join the
    // volume eases back to as set; the new section's first notes still play.
    add(1, -32, 32, ::variations::fade_out, 255, 0x01);
    entry = 0, next_entry = 1;
    clock_.position = clock_.carry = 0;
    clock_.play();
    pass(), pass();
    while (entry == 0) pass();
    require(entry == 1, "the join");
    {
        opxy::i32 last = gain();
        require(last > 0 && last < kFull / 4, "the ease back starts a pass ahead of the join");
        Context c = now();
        require(note(c, 0), "the new section's first note plays (an ease never drops notes)");
        opxy::u32 steps = 0;
        while (gain() != kFull) {
            pass();
            // Build 074: a fade to silence climbs back over 32 passes (46 ms), not 6: steps of a 32nd.
            require(gain() > last && gain() - last <= kFull / 32 + 0x01000000, "eases up in small steps");
            last = gain();
            require(++steps <= 32, "back within 32 passes");
        }
        require(steps >= 28, "the climb out of a fade takes about 32 passes, not the ease in's 6");
    }
    stop();
    std::puts("variations play start: ok");
    return 0;
}
