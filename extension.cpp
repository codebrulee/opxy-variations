// Variations (notes/variations-format.md): plays the project's variations.
// Saved with the project (saved state); each sequencer pass turns the lanes'
// targets into track values (mixer.hpp); each written note on tracks 1-8
// passes a lane on the notes (notes.hpp). The variation page (page.hpp,
// M4 on the song page, its label "variation" over M4 while Shift is up)
// places them and picks their gesture, and the song page marks the joins that
// hold one (song_marks);
// tools/variations.py writes them from the computer.
#include <opxy/extension.hpp>
#include "mixer.hpp"
#include "notes.hpp"
#include "page.hpp"
#include "retrigger.hpp"

namespace opxy { namespace extensions { namespace variations {
namespace {
using namespace ::variations;
// Constant-initialised: the stock loader runs no static constructors.
Store store;
Mixer mixer;
Ranking rankings[kTracks];
PatternNotes pattern;  // read_notes' copy; 1.5 KB, kept off the stack
Targets targets_now;
u32 project = 0;
Heard heard;  // the tracks heard in each scene: the page's default tracks
VariationPage page_model;

// How the song came to the entry playing (window.hpp's scene guard),
// followed once a pass: the scene the entry before it played, when the song
// came straight from it; and whether playback (re)started there instead: Play
// (the first pass running), a jump (not from the entry before), or the song's
// start (entry 0, from Play or as the song loops round).
struct Came {
    bool seen;
    u32 song, entry, scene, before;
    bool restarted;
    void pass(const Context& c) {
        const opxy::extensions::SongPosition& s = c.song;
        if (!s.running || s.song == no_song || !s.entries) {
            seen = false, before = kNoScene, restarted = false;
            return;
        }
        if (!seen || s.song != song) {
            before = kNoScene, restarted = true;
        } else if (s.entry != entry) {
            const bool stepped = (entry + 1) % s.entries == s.entry;
            before = stepped ? scene : kNoScene;
            restarted = !stepped || s.entry == 0;
        }
        seen = true, song = s.song, entry = s.entry, scene = s.scene;
    }
};
Came came = {false, 0, 0, 0, kNoScene, false};

// This pass's advance of the scene clock (ticks), from the last pass's
// position: the next pass is taken to go as far (ahead(), window.hpp). Kept
// over a scene boundary and while nothing new is known; 0 (no lead) until two
// running passes have been seen.
struct Pace {
    bool seen;
    u32 entry, position, advance;
    void pass(const Context& c) {
        const opxy::extensions::SongPosition& s = c.song;
        if (!s.running) {
            seen = false, advance = 0;
            return;
        }
        if (seen && s.entry == entry && s.position > position && s.position - position <= kStepTicks)
            advance = s.position - position;
        seen = true, entry = s.entry, position = s.position;
    }
};
Pace pace = {false, 0, 0, 0};
// The eases before and after a window (window.hpp ease_targets): this many
// passes (audio blocks, about 9 ms) at the measured pace; none until it is known.
const u32 kEasePasses = 6;
// Build 074: the climb back to as set after a window whose end is far from it (a fade
// out), at the most: 32 passes, about 46 ms (a pass is 1.45 ms). The ease in only has to
// avoid a one-block cut (6 passes, 8.7 ms); a full-scale volume climb in 8.7 ms was
// audible as a click on a ringing tail. Smaller jumps take proportionally less
// (window.hpp out_length), down to kEasePasses.
const u32 kEaseOutPasses = 32;

// Where Play will start, while the scene clock doesn't run (window.hpp
// play_start), to judge Play's first notes (nothing is sent there while
// stopped): stopped, the entry the next boundary starts, which the SDK
// gives as Play's own (stock's Play starts the song's first entry unless the
// song page moved the next one, then runs that boundary step); in a count-in,
// Play has already taken it and it is the entry playing. Nothing with song playback off, no song, or a scene
// cued (it plays first, and the song waits). Kept from the last pass for
// Play's first notes, which still see the stopped position.
struct Start {
    bool known;
    ::variations::SongPosition at;
    void pass(const Context& c) {
        const opxy::extensions::SongPosition& s = c.song;
        known = !s.running && s.song != no_song && s.entries && s.queued == no_scene;
        if (!known) return;
        const u32 entry = c.playing ? s.entry : s.next_entry;
        at = play_start(s.song, entry < s.entries ? entry : 0);
    }
};
Start start = {};

// Build 052, superseded by build 076 below (2026-11-24: press Stop while a variation holds a track silent
// and that track's tail rings out at full level). Stop used to give every held
// word back on its first stopped pass, with no easing, so a track whose volume a
// lane held at or below kSilent jumped to full level in one pass while the voice
// stock had just released was still decaying: the tail became audible. Stock's
// own Stop never does this, because nothing was holding the level down. Now the
// volume words a lane was holding at or below kSilent are kept held on the
// stopped passes (every other word is given back at once, as before) until the
// first of:
//   - any live note input (the SDK's input_activity: any track, accepted or
//     not): the tail is gone by the time a key is played, and the key must
//     sound as set;
//   - kHoldPasses stopped passes (the backstop, so nothing is ever stuck silent
//     when nothing is played);
//   - Play (the running branch recomputes the targets; a count-in does the same
//     through its start targets), a project change (mixer.release) or a load.
// There is no clock in the pass context (Context has the song position only,
// and it reads 0 while stopped), so the backstop counts stopped passes. A pass
// is one audio block: 64 frames at 44100 Hz, 689 a second (notes/opxy-facts.md,
// probe 086; docs/extensions.md), so about 1.45 ms. (Notes elsewhere say "a
// pass, about 9 ms": that is kEasePasses = 6 passes, 8.7 ms.)
// Accepted residual: a key struck on a held-silent track inside the hold window
// only becomes audible on the next pass, so its first pass (one block, about
// 1.45 ms) is quiet.
// The drives are recorded during the running passes (a small fixed record: the
// pass is a realtime callback, nothing is allocated).
// Build 076 (2026-11-25, on build 074: "it now jumps immediately back to full
// volume or equivalent based on your target...which I guess makes sense but is quite
// jarring, especially if you were listening at high volume...I'd expect it to hold
// whatever volume it was at, even if that was 0, but reset when you hit play, or play
// the keyboard etc."; asked when, he said: when he presses Stop). Stop now holds EVERY
// driven word where the gesture left it, not only the silent volumes: nothing is sent on
// the stopped passes (the build 052 silent-only hold and its one-second backstop are
// gone). The hold ends at the first of: live note input (input_activity, as in build
// 052), Play (the running branch recomputes the targets; a count-in does the same through
// its start targets), a project change or a load (mixer.release / clear).
// `holding` is set by every running pass; `fresh` marks the first stopped pass, which
// forgets a key struck between the last running pass and the Stop.
struct SilentHold {
    bool holding;          // a running pass has driven words: a Stop keeps them held
    bool fresh;            // no stopped pass yet since the last running one
    volatile bool played;  // input_activity: a live note arrived (cleared by each running pass)
    void clear() { holding = false, fresh = false, played = false; }
    void running() { holding = true, fresh = true, played = false; }
    // A stopped pass: whether the words stay held.
    bool keep() {
        if (!holding) return false;
        if (fresh) fresh = false, played = false;
        if (played) {
            holding = false, played = false;
            return false;
        }
        return true;
    }
};
SilentHold silent_hold = {};
// Cheap on purpose: the next pass acts on it.
void input_activity() { silent_hold.played = true; }

// The SDK's song position as the window needs it.
::variations::SongPosition where(const Context& c) {
    const opxy::extensions::SongPosition& s = c.song;
    ::variations::SongPosition p = {};
    p.playing = s.running && s.song != no_song;
    p.song = s.song;
    p.entry = s.entry;
    p.next_entry = s.next_entry;
    p.position = s.position;
    p.length = s.length;
    p.scene = s.scene;
    p.scene_before = came.before;
    p.restarted = came.restarted;
    p.stops = !s.loops && s.entries && s.entry + 1 == s.entries && s.next_entry == 0;
    return p;
}

// The mixer's view of the SDK, for the callback in progress.
const Context* pass_context;
const Actions* pass_actions;
bool io_read(u32 t, u32 w, i32* v) { return pass_actions->read_track_value(*pass_context, t, w, *v); }
bool io_filter(u32 t, u32* type, bool* on) {
    TrackFilter f;
    if (!pass_actions->read_track_filter(*pass_context, t, f)) return false;
    *type = f.type;
    *on = f.on;
    return true;
}
bool io_send(u32 t, u32 w, i32 v) { return pass_actions->send_track_value(*pass_context, t, w, v); }
// Whether the track's engine has an engine control (build 024: a lane's
// tracks may run different engines, a drum track beside a synth; each moves
// its own engine's control, and a track whose engine has no such slot is left
// alone). Named means there; where no name can be read at all (not even the
// volume's), the slot is driven as before.
volatile u32 named_word = ::variations::kVolume;  // a word every track names (volatile: GCC's bounds check would see through the label's fixed name)
bool io_has(u32 t, u32 w) {
    char text[track_label_capacity];
    if (!pass_actions->track_target_label || pass_actions->track_target_label(*pass_context, t, w, text, sizeof text))
        return true;
    return !pass_actions->track_target_label(*pass_context, t, named_word, text, sizeof text);
}
const TrackIo io = {io_read, io_filter, io_send, io_has};
// Build 025: the lanes' engine controls placed on each track's engine by slot
// ("param 1") or by name ("tone"), by the names the SDK gives (engine.hpp).
bool pass_label(u32 t, u32 w, char* text, u32 capacity) {
    return pass_actions->track_target_label && pass_actions->track_target_label(*pass_context, t, w, text, capacity);
}
Engines pass_engines() { return Engines{pass_label, heard.percussion}; }

void sequencer_pass(const Context& c, const Actions& a) {
    pass_context = &c;
    pass_actions = &a;
    if (c.recording.project != project) {
        // Another project: give every word we held back first (stock sends its
        // own value again only when that value changes).
        project = c.recording.project;
        mixer.release(io);
        silent_hold.clear();
        for (Ranking& r : rankings) r.forget();
        heard.forget();
        came.seen = false;
        pace.seen = false, pace.advance = 0;
    }
    // The first pass running after Play: everything of ours goes again, in case
    // stock sent its own values as the transport started.
    const bool started = c.song.running && !pace.seen;
    came.pass(c);
    pace.pass(c);
    start.pass(c);
    page_model.follow(where(c));  // the page's playhead (nothing while it is closed)
    if (!c.song.running) {
        // Stopped: every word is stock's own, so the keys sound as set while
        // the transport rests (a fade in on the song's start doesn't hold its
        // tracks silent then); anything held is given back. Play's first notes
        // on a track a lane holds silent are dropped instead (sequenced_note),
        // and its first running pass sends ours. Counting in, the transport
        // already plays: the values where Play started are in place before the
        // clock runs.
        // Build 052: except the volumes a lane held silent (see SilentHold),
        // which stay held until a note is played or the backstop. A count-in
        // (Play pressed again) takes over with its own start targets.
        targets_now.count = 0;
        const Engines engines = pass_engines();
        if (c.playing && start.known) {
            silent_hold.clear();
            targets(store, start.at, &targets_now, 0, &engines);
        } else {
            // Build 076 ("hold whatever volume it was at, even if that was 0"):
            // held, nothing is sent; once a key is played the words go back in one
            // step (the key's own onset covers it).
            if (silent_hold.keep()) return;
        }
        mixer.apply(targets_now, io);
        return;
    }
    // One pass ahead: in place before the next pass's notes start; eased in
    // before a window and out after one.
    const u32 next_scene = c.song.queued == no_scene ? kNoScene : c.song.queued;
    if (started) mixer.refresh();
    const Engines engines = pass_engines();
    targets(store, ahead(where(c), pace.advance, next_scene), &targets_now, kEasePasses * pace.advance, &engines,
            kEaseOutPasses * pace.advance);
    silent_hold.running();  // build 076: a Stop after this pass holds what it drove
    mixer.apply(targets_now, io);
}

bool sequenced_note(const Context& c, SequencedNote& n, const Actions& a) {
    if (n.track >= kTracks) return true;
    if (c.song.length > c.song.position) heard.note(c.song.scene, n.track, n.percussion, c.song.length - c.song.position);
    if (!store.count()) return true;
    Targets t;
    // Play's first pass: its notes start before the clock reads as running,
    // so they are judged where Play starts (start, from the stopped passes).
    // Build 045 (2026-11-23: a drop to silence clicked at random, at least
    // once, often on a downbeat): while running, a note is judged a pass ahead,
    // where the volumes sent are. Its context can be a pass stale, so judged
    // where it reads, a note at a drop's start sounded loud, started into the
    // silence already sent, and was cut at once. A pass ahead, a note on a
    // boundary is judged on the boundary's far side: dropped at a cut, played
    // at a gate's "on" step (gate).
    ::variations::SongPosition judged;
    if (c.song.running) {
        const u32 next_scene = c.song.queued == no_scene ? kNoScene : c.song.queued;
        judged = ahead(where(c), pace.advance, next_scene);
    } else if (start.known) {
        judged = start.at;
    } else {
        return true;
    }
    targets(store, judged, &t);
    // A lane holding the track's volume silent where the note starts: the note
    // doesn't sound at all (a build's track before its entry, a break's after
    // its exit, a drop's gap), rather than for a moment before the value lands.
    u32 level;
    if (t.find(n.track, kVolume, &level) && level <= kSilent) return false;
    // Build 046: a stutter lane's window covering the note retriggers it.
    Stutter stutter = {};
    const bool stuttering = find_stutter(store, judged, n.track, &stutter);
    const bool line = t.notes_set[n.track] && t.notes[n.track] != kMiddle;
    if (!line && !stuttering) return true;
    Ranking& r = rankings[n.track];
    if (!a.read_notes(c, n.track, pattern) || !r.refresh(pattern)) return true;
    if (line) {
        const i32 rank = r.rank(n.written);
        if (rank >= 0) {
            const NoteChoice choice = choose(t.notes[n.track], static_cast<u32>(rank), r.groups(), r.gap(n.written), n.length, n.velocity);
            if (!choice.play) return false;
            if (choice.extra) {
                const ExtraNote extra = {choice.delay, n.written.pitch, choice.velocity, choice.length};
                if (a.play_note(c, extra) && choice.own_length < n.length) n.length = choice.own_length;
            }
        }
    }
    if (stuttering) {
        // Build 047: the lane's line is the density; retrigger_plan walks it (retrigger.hpp).
        const u32 gap = r.gap(n.written), room = gap < stutter.remaining ? gap : stutter.remaining;
        Repeat plan[kMaxRepeats] = {};
        const u32 repeats = retrigger_plan(stutter, n.velocity, n.length, room, plan);
        if (repeats) {
            for (u32 k = 0; k < repeats; ++k) {
                const ExtraNote again = {plan[k].delay, n.written.pitch, plan[k].velocity, plan[k].length};
                if (!a.play_note(c, again)) break;
            }
            if (plan[0].delay < n.length) n.length = plan[0].delay;  // the note itself ends where the first repeat starts
        }
    }
    return true;
}

u32 save(u8* data, u32 capacity) {
    const u32 n = store.size();
    if (n > capacity) return 0;
    for (u32 i = 0; i < n; ++i) data[i] = store.bytes()[i];
    return n;
}
void load(const u8* data, u32 bytes) {
    if (!store.load(data, bytes)) store.clear();
    for (Ranking& r : rankings) r.forget();
    heard.forget();
    silent_hold.clear();  // build 052: the drives were of the store just replaced
}

// Every song's variations kept on their joins (store.hpp repin): the page's
// open is where read_song is granted, so song edits are caught up here.
SongEntry song_entries[kEntries];
u8 song_scenes[96];
u32 song_steps[96];
void repin_all(const Context& c, const Actions& a) {
    for (u32 song = 0; song < 14; ++song) {
        bool any = false;
        for (u32 i = 0; i < store.count() && !any; ++i) any = store[i].song == song;
        if (!any) continue;
        const u32 n = a.read_song(c, song, 0, song_entries, 96);
        if (!n) continue;  // empty (or not read): nothing plays there; leave them be
        for (u32 j = 0; j < n; ++j) {
            song_scenes[j] = static_cast<u8>(song_entries[j].scene < kSceneCount ? song_entries[j].scene : kNoScene);
            song_steps[j] = song_entries[j].length / kStepTicks;
        }
        repin(store, song, song_scenes, n);
        // Build 047: a long-form window's reach follows the song's sections as they are now.
        reach_repin(store, song, song_scenes, song_steps, n);
    }
}

// The page: the song around the join under the song page's cursor, and the
// whole song's shape, so - and + can move the page to other joins.
SongShape shape;
u32 opened_join = 0;   // the song page's cursor as the page opened
bool returning = false;  // the SDK was asked to leave the cursor elsewhere
// The page's names for targets on tracks (Actions::track_target_label), for
// the callback in progress.
const Context* page_context;
const Actions* page_actions;
bool target_name(u32 track, u32 word, char* text, u32 capacity) {
    return page_actions && page_actions->track_target_label &&
           page_actions->track_target_label(*page_context, track, word, text, capacity);
}
const Names names = {target_name};
struct PageCallback {
    PageCallback(const Context& c, const Actions& a) { page_context = &c, page_actions = &a; }
    ~PageCallback() { page_context = nullptr, page_actions = nullptr; }
};
bool page_open(const Context& c, const SongCursor& cursor, const Actions& a) {
    PageCallback call(c, a);
    repin_all(c, a);
    opened_join = cursor.join;
    returning = false;
    Around around = {};
    shape.entries = 0;
    if (cursor.song != no_song && cursor.entries) {
        const u32 n = a.read_song(c, cursor.song, 0, song_entries, kEntries);
        if (n && n == cursor.entries) {
            shape.entries = n;
            for (u32 j = 0; j < n; ++j) {
                shape.scene[j] = static_cast<u8>(song_entries[j].scene < kSceneCount ? song_entries[j].scene : kNoScene);
                shape.steps[j] = song_entries[j].length / kStepTicks;
            }
            around.song = true;
            around.song_index = cursor.song;
            around.entries = n;
            around.entry = cursor.join < n ? cursor.join : 0;  // the end joins into entry 1
            const u32 before = (around.entry + n - 1) % n;
            around.scene_after = song_entries[around.entry].scene;
            around.steps_after = shape.steps[around.entry];
            around.scene_before = song_entries[before].scene;
            around.steps_before = shape.steps[before];
            around.shape = &shape;
            // Build 025: a song that doesn't loop (the song playing is this
            // one: its loops flag) has a start and an end, not a loop point.
            around.stops = c.song.song == cursor.song && !c.song.loops;
            around.at_end = cursor.join >= n;
        }
    }
    // No song to open on (song playback off, an empty song): the page stays
    // closed, so M4 does nothing (the song page shows no label then either).
    if (!around.song) return false;
    page_model.open(store, around, heard, &names);
    return true;
}
// After - or + moved the page to another join, closing it (the gesture or a
// delete) leaves the song page's cursor on the join shown: the join into
// entry j is the cursor j, the one into entry 0 the song's end (where the
// song loops back). Back on the join it opened on: where it was.
void follow_join(const Actions& a) {
    if (!a.return_to) return;
    if (page_model.moved()) {
        // The join into entry 1: the song's end, or (build 025) its start
        // when the song doesn't loop and the window is at the start.
        a.return_to(page_model.entry() ? page_model.entry() : page_model.at_song_start() ? 0 : shape.entries);
        returning = true;
    } else if (returning) {
        a.return_to(opened_join);
        returning = false;
    }
}
// A delete closes the page; the SDK shows its notice (deleted) once the page
// has closed, over the song page.
PageAnswer page_input(const Context& c, const PageInput& in, const Actions& a) {
    PageCallback call(c, a);
    const bool closing = page_model.input(store, in, &names);
    follow_join(a);
    if (!closing) return PageAnswer::stay;
    if (const char* notice = page_model.notice())
        if (a.notify) a.notify(notice);
    return PageAnswer::close;
}
void page_leds(PageLeds& out) { page_model.leds(out); }
void page_close(const Context&, PageClose) { page_model.close(store); }
}
extern constexpr Hooks hooks = Hooks().with_input_activity(input_activity).with_sequencer_pass(sequencer_pass).with_sequenced_note(sequenced_note);
extern constexpr SavedState saved_state = {save, load};
extern constexpr Page page = Page().with_open(page_open).with_input(page_input).with_leds(page_leds).with_close(page_close);
// The song page's marks: a dotted line on each join of the song shown that holds a
// placed variation, found by its scenes (store.hpp join_of), so a song edited
// since the page last opened still shows each variation where the page will
// put it.
void song_marks(const SongView& view, SongMarks& marks) {
    song_marks_of(store, view.song, view.scenes, view.entries, marks);
}
using PageFrame = ::variations::PageFrame;
void page_draw(const PageTick& tick, PageFrame& frame) { page_model.draw(tick, frame); }
}}}
