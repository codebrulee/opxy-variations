// Host test: the page through the extension's own callbacks (extension.cpp):
// opening on vartest's join with read_song, an edit heard at once on the
// scene clock's next passes, kept by saved state, - and + on to another join, and a
// variation deleted with Shift + M4 (build 021; the page closes, "deleted" said
// through Actions::notify), the Shift layer, a lane's target picked by
// encoder 3 and named by Actions::track_target_label, the header's number in song order, and the song
// page's cursor asked to follow the join shown after - and + (Actions::return_to).
#include "../extension.cpp"
#include "texts.hpp"
#include <opxy/extension_testing.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
using namespace opxy::extensions;
namespace vx = opxy::extensions::variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
// examples/first-test.json (vartest's variations) as build 003 saved them
// (version 1): three variations once rewritten as version 6.
static const opxy::u8 vartest[] = {86, 82, 1, 2, 0, 1, 224, 255, 32, 0, 0, 0, 3, 0, 0, 2, 16, 2, 0, 128, 0, 31, 0, 0, 33, 2, 0, 0, 0, 31, 255, 40, 0, 1, 32, 0, 32, 0, 0, 0, 12, 0, 0, 3, 50, 2, 0, 128, 0, 31, 0, 196, 3, 2, 0, 255, 0, 31, 0, 0, 19, 3, 0, 128, 0, 15, 255, 0, 31, 255, 0};
// Song 1: scene 1, scene 2, 4 bars each (until the song edits at the end).
static opxy::u32 song_scenes[8] = {0, 1};
static opxy::u32 song_length = 2;
static opxy::u32 read_song(const Context&, opxy::u32 song, opxy::u32 first, SongEntry* out, opxy::u32 count) {
    opxy::u32 n = 0;
    for (; song == 0 && first + n < song_length && n < count; ++n) out[n] = SongEntry{song_scenes[first + n], 64 * 480};
    return n;
}
static void song_is(std::initializer_list<opxy::u32> scenes) {
    song_length = 0;
    for (opxy::u32 sc : scenes) song_scenes[song_length++] = sc;
}
static testing::SceneClock clock_;
static testing::TrackValues values;
static opxy::u32 entry = 0;
static bool loops = true;  // the song's loop flag (build 025: off, it stops at its end)
static Context now() {
    SongPosition s = {};
    s.song = 0, s.entry = entry, s.entries = song_length, s.next_entry = (entry + 1) % song_length, s.loops = loops;
    s.scene = song_scenes[entry];
    Context c = {};
    c.recording.project = 1;
    c.playing = clock_.playing;
    c.song = clock_.after_pass(s);
    c.page = pages::extension;
    return c;
}
static bool pass() {
    const bool ended = clock_.pass(20);
    if (ended) entry = (entry + 1) % song_length;
    values.pass(vx::hooks, now());
    return ended;
}
static PageInput turn(opxy::u32 encoder, opxy::i32 step) {
    PageInput in = {};
    in.kind = PageInput::turn, in.encoder = encoder, in.step = step;
    return in;
}
static opxy::u8 saved[8192];
static char notified[32];
static opxy::u32 notifications;
static void notify(const char* text) {
    std::strncpy(notified, text, sizeof notified - 1);
    ++notifications;
}
// Actions::return_to: the join the song page's cursor goes to when the page closes itself.
static opxy::u32 returned_to, returns;
static void return_to(opxy::u32 join) {
    returned_to = join;
    ++returns;
}
int main() {
    // Build 052: the strip fits 55 px at 15 px by the device's measure (texts_check::device_like); the estimate is wider.
    opxy::draw::host_text_measure() = texts_check::device_like;
    const SavedState& st = vx::saved_state;
    const Page& page = vx::page;
    Actions actions = {};
    actions.read_song = read_song;
    actions.notify = notify;
    actions.return_to = return_to;
    actions = values.actions(actions);  // track_target_label: the SDK's double
    values.names[0][0] = "Shape";
    st.load(vartest, sizeof vartest);
    const opxy::u32 upgraded = st.save(saved, sizeof saved);
    require(upgraded > sizeof vartest && saved[2] == 12 && saved[3] == 3, "vartest as version 12: three variations");
    for (opxy::u32 t = 0; t < 8; ++t) {
        values.stock_sets(t, track_word::gain, 0x7fffffff);
        values.stock_sets(t, track_word::fx1_send, 0x19999999);
        values.stock_sets(t, track_word::cutoff, 0x5999999a);
        values.filter[t] = TrackFilter{16, true};
    }
    // The extension hears tracks 1-3 in scene 1 (a note mid-scene).
    clock_.length = clock_.rule = 64 * 480;
    clock_.play();
    for (opxy::u32 i = 0; i < 100; ++i) pass();
    for (opxy::u32 t = 0; t < 3; ++t) {
        SequencedNote note = {};
        note.track = t, note.percussion = t < 2, note.length = 240, note.velocity = 100;
        Context c = now();
        c.song.running = false;  // no window: the note is only heard
        c.song.position = 4800;
        require(vx::hooks.sequenced_note(c, note, actions), "notes pass");
    }
    // Open on the join between scene 1 and scene 2: vartest's first variation.
    SongCursor cursor = {0, 1, 2};
    require(page.open(now(), cursor, actions), "opens");
    static vx::PageFrame frame;
    vx::page_draw(PageTick{16, true}, frame);
    require(opxy::draw::valid(frame), "a valid first frame");
    require(!std::strcmp(frame.texts[::variations::screen::t_value_2].chars, "edit"), "an edited line: vartest's first (build 053: \"edit\", was \"custom\")");
    require(!std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "notes"), "lane a: its notes line");
    // Encoder 2: break (any turn leaves custom), then two on: drop, on its tracks 1
    // and 2 (amount 0: a beat of silence before the join); encoder 3 on to the
    // volume (past the engine's controls, named by the SDK, and the envelope).
    page.input(now(), turn(1, 1), actions);
    page.input(now(), turn(1, 2), actions);
    // (Build 024: vartest's custom lane a is on track 1 alone, the track with points; track 2 added.)
    if (!(vx::page_model.draft().lanes[0].selection & 2)) {
        PageInput key = {};
        key.kind = PageInput::press, key.key = keys::track + 1;
        page.input(now(), key, actions);
    }
    require(vx::page_model.draft().lanes[0].selection == 3, "the drop on tracks 1 and 2");
    PageInput target = turn(2, 1);
    page.input(now(), target, actions);
    vx::page_draw(PageTick{16, false}, frame);
    // Build 025: param 1, slot 1 on both tracks whatever each engine calls it (track 1's
    // engine names it "shape", track 2's gives no name: its position, left alone where it plays).
    require(!std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "par 1"), "param 1 first after the notes (build 053: \"par 1\" in the strip, 54 px whole)");
    require(!std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "par 1"), "the name alone, no place");
    page.input(now(), target, actions);
    vx::page_draw(PageTick{16, false}, frame);
    require(!std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "par 2"), "param 2 (build 053: \"par 2\")");
    // Build 032: a two-track lane lists "all" (no engine's own names: those need the lane
    // narrowed to one track): on to the volume without meeting track 1's "shape".
    for (opxy::u32 i = 0; i < 20; ++i) {
        page.input(now(), target, actions);
        vx::page_draw(PageTick{16, false}, frame);
        require(std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "shape") != 0,
                "no engine's own name for a lane of two tracks");
        if (!std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "vol")) break;  // build 053: the strip's short name
    }
    PageInput up = {};
    up.kind = PageInput::release, up.key = keys::shift;
    page.input(now(), up, actions);
    vx::page_draw(PageTick{16, false}, frame);
    require(frame.texts[::variations::screen::t_value_2].chars[0] == 'd' &&
                !std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "vol"),
            "drop, on the volume (build 053: \"vol\")");
    // Heard at once: scene 1 plays on into the window; tracks 1 and 2 cut at step 60.
    while (clock_.position < 56 * 480) pass();
    require(values.dsp[0][track_word::gain] == 0x7fffffff, "before the cut: full");
    {
        namespace scr = ::variations::screen;
        vx::page_draw(PageTick{16, false}, frame);
        // Build 049: the timeline's playhead is the plot-high hairline on the cursor's node; the shape page's mark stays hidden.
        const opxy::draw::Node& head = frame.nodes[scr::n_cursor];
        require(head.x != opxy::draw::hidden_x * opxy::draw::unit && head.shade == scr::s_white &&
                    frame.nodes[scr::n_playhead].x == opxy::draw::hidden_x * opxy::draw::unit,
                "the playhead on the timeline, white inside the window (from the sequencer pass)");
    }
    while (clock_.position < 61 * 480) pass();
    require(values.dsp[0][track_word::gain] == 0 && values.dsp[1][track_word::gain] == 0, "the drop: silent");
    require(values.dsp[2][track_word::gain] == 0x7fffffff, "track 3 untouched");
    while (!pass()) {}
    require(values.dsp[0][track_word::gain] == 0x7fffffff, "back at the join");
    // Saved with the project, and loaded again.
    const opxy::u32 n = st.save(saved, sizeof saved);
    require(n > 0 && saved[3] == 3 && saved[4 + 9] == 3 && saved[4 + 11] == track_word::gain,
            "saved: three, the first a drop on the volume");
    page.close(now(), PageClose::back);
    st.load(saved, n);
    require(page.open(now(), cursor, actions), "reopens");
    vx::page_draw(PageTick{16, true}, frame);
    require(frame.texts[::variations::screen::t_value_2].chars[0] == 'd', "the drop again after a reload");
    page.close(now(), PageClose::left);
    // The end of the song joins into entry 1: a new one there (vartest has none).
    SongCursor end = {0, 2, 2};
    require(page.open(now(), end, actions), "opens at the end");
    require(st.save(saved, sizeof saved) > n, "the new one is kept at once");
    vx::page_draw(PageTick{16, true}, frame);
    require(!std::strcmp(frame.texts[::variations::screen::t_title].chars, "variation 1 of 4 new"), "first in the song (its start), new");
    page.close(now(), PageClose::left);
    // Opened on the scene 1 > scene 2 join again, + + steps on to it (the song read at open).
    require(returns == 0, "no jump so far: the cursor is never asked to move");
    require(page.open(now(), cursor, actions), "opens on vartest's join");
    PageInput plus = {};
    plus.kind = PageInput::press, plus.key = keys::plus;
    PageInput minus = plus;
    minus.key = keys::minus;
    page.input(now(), plus, actions);
    page.input(now(), plus, actions);
    require(returns == 0, "+ to the join's second and third: the same join, nothing asked");
    page.input(now(), plus, actions);
    require(returns == 1 && returned_to == 2, "+ on to the end join: the cursor to follow there (the song's end)");
    page.input(now(), minus, actions);
    require(returns == 2 && returned_to == 1, "- back to the join it opened on: the cursor where it was");
    page.input(now(), turn(3, 1), actions);
    require(returns == 2, "unmoved: nothing more asked");
    page.input(now(), plus, actions);
    require(returns == 3 && returned_to == 2, "+ again: the end join");
    page.input(now(), turn(3, -1), actions);  // the amount as it was
    require(returns == 4 && returned_to == 2, "each input keeps it there");
    vx::page_draw(PageTick{16, false}, frame);
    require(!std::strcmp(frame.texts[::variations::screen::t_title].chars, "variation 1 of 4") && !std::strcmp(frame.texts[::variations::screen::t_join].chars, "scene 2 > end") &&
                frame.texts[::variations::screen::t_value_2].chars[1] == 'r',
            "+ +: on to the song's end join, its BREAK (build 024: before the song's start, \"scene 2 > end\")");
    require(!std::strcmp(frame.texts[::variations::screen::t_label_a].chars, "scene 2, end") &&
                !std::strcmp(frame.texts[::variations::screen::t_label_b].chars, "start, scene 1"),
            "the scene names along the plot: where the song ends and starts");
    // Shift held: the Shift layer's names along the bottom.
    PageInput shift = {};
    shift.kind = PageInput::press, shift.key = keys::shift, shift.shift = true;
    require(page.input(now(), shift, actions) == PageAnswer::stay, "Shift: the page stays");
    vx::page_draw(PageTick{16, false}, frame);
    require(!std::strcmp(frame.texts[::variations::screen::t_name_1].chars, "snap") &&
                (!std::strcmp(frame.texts[::variations::screen::t_name_2].chars, "until") ||
                 !std::strcmp(frame.texts[::variations::screen::t_name_2].chars, "beats") ||
                 !std::strcmp(frame.texts[::variations::screen::t_name_2].chars, "bar") ||
                 !std::strcmp(frame.texts[::variations::screen::t_name_2].chars, "bars")) &&
                frame.texts[::variations::screen::t_value_2].chars[0] &&
                !std::strcmp(frame.texts[::variations::screen::t_name_3].chars, "rate") &&
                !std::strcmp(frame.texts[::variations::screen::t_value_3].chars, "1x") &&
                !frame.texts[::variations::screen::t_name_4].chars[0] &&
                !std::strcmp(frame.texts[::variations::screen::t_mark].chars, "delete"),
            "the Shift layer: snap, length, rate, and \"delete\" over M4 (build 030, 041; 052: the mark slot, not column 4)");
    // Build 020's Shift + encoder 3 and 4 presses (the shape page, delete) and 013's Shift + encoder 2: nothing now.
    for (opxy::u32 e = 1; e < 4; ++e) {
        PageInput old = {};
        old.kind = PageInput::click, old.encoder = e, old.key = keys::encoder + e, old.shift = true;
        require(page.input(now(), old, actions) == PageAnswer::stay && notifications == 0, "Shift + an encoder's press: nothing");
    }
    PageInput shift_up = shift;
    shift_up.kind = PageInput::release, shift_up.shift = false;
    page.input(now(), shift_up, actions);
    // Encoder 2's tap (build 022; 021's encoder 4): the shape page; Shift + M4 there does nothing (build 030; never the variation); the tap again: back.
    PageInput tap_2 = {};
    tap_2.kind = PageInput::click, tap_2.encoder = 1, tap_2.key = keys::encoder + 1;
    PageInput del = {};
    del.kind = PageInput::press, del.key = keys::m1 + 3, del.shift = true;
    require(page.input(now(), tap_2, actions) == PageAnswer::stay, "the shape page: the page stays");
    vx::page_draw(PageTick{16, false}, frame);
    if (!std::strcmp(frame.texts[::variations::screen::t_name_1].chars, "step")) {
        require(page.input(now(), del, actions) == PageAnswer::stay && notifications == 0,
                "Shift + M4 on the shape page: the page stays, nothing deleted");
        page.input(now(), shift_up, actions);
        require(page.input(now(), tap_2, actions) == PageAnswer::stay, "back: the page stays");
        vx::page_draw(PageTick{16, false}, frame);
    }
    require(!std::strcmp(frame.texts[::variations::screen::t_name_1].chars, "move"),
            "back on the variation page");
    // Shift + M4 there (stock's song page delete): deleted, the page answers close and asks for "deleted".
    require(page.input(now(), del, actions) == PageAnswer::close && returned_to == 2,
            "a delete closes the page, the cursor to follow to the join shown");
    require(notifications == 1 && !std::strcmp(notified, "deleted"), "deleted, to be said over the song page");
    require(st.save(saved, sizeof saved) == n, "deleted at once");
    page.close(now(), PageClose::done);
    // The SDK leaves the song page's cursor on the end join; reopened on vartest's join here.
    require(page.open(now(), cursor, actions), "reopens");
    vx::page_draw(PageTick{16, true}, frame);
    require(!std::strcmp(frame.texts[::variations::screen::t_title].chars, "variation 1 of 3") && !std::strcmp(frame.texts[::variations::screen::t_join].chars, "scene 1 > scene 2"), "on the cursor's join");
    page.close(now(), PageClose::back);
    require(st.save(saved, sizeof saved) == n, "nothing new");
    // Song playback off, or an empty song: the page stays closed (M4 does nothing); nothing is kept.
    SongCursor off = {no_song, 0, 0}, empty = {0, 0, 0};
    require(!page.open(now(), off, actions) && !page.open(now(), empty, actions), "closed without a song");
    require(st.save(saved, sizeof saved) == n, "nothing changed");
    // A project load under the page closes it as `project`: the store stays the loaded one.
    require(page.open(now(), cursor, actions), "opens");
    st.load(vartest, sizeof vartest);
    page.close(now(), PageClose::project);
    require(st.save(saved, sizeof saved) == upgraded, "the loaded project's variations");

    // Song edits. vartest as build 003 left it: its song now scene 1, scene 1, scene 2 (an entry
    // inserted before the join its variations were made on), its variations version 1 (scenes
    // not known), still at the join into entry 2: now between the two scene 1s.
    clock_.stop();
    values.pass(vx::hooks, now());
    st.load(vartest, sizeof vartest);
    song_is({0, 0, 1});
    SongCursor ones = {0, 1, 3};
    require(page.open(now(), ones, actions), "opens between the scene 1s");
    vx::page_draw(PageTick{16, true}, frame);
    namespace scr = ::variations::screen;
    require(!std::strcmp(frame.texts[scr::t_title].chars, "variation 1 of 3") && !std::strcmp(frame.texts[scr::t_join].chars, "scene 1 > scene 1"), "all three there, not moved");
    page.close(now(), PageClose::back);
    opxy::u32 m = st.save(saved, sizeof saved);
    require(m == upgraded && saved[2] == 12 && saved[4 + 1] == 1 && saved[4 + 7] == 0 && saved[4 + 8] == 0,
            "their scenes filled in place: 1 > 1");
    // Unchanged: reopening moves nothing.
    static opxy::u8 again[8192];
    require(page.open(now(), ones, actions), "reopens");
    page.close(now(), PageClose::back);
    require(st.save(again, sizeof again) == m && std::memcmp(saved, again, m) == 0, "unchanged song: same bytes");
    // An entry inserted at the front (scene 2): they follow to the join into entry 3.
    song_is({1, 0, 0, 1});
    SongCursor moved = {0, 2, 4};
    require(page.open(now(), moved, actions), "opens on the moved join");
    vx::page_draw(PageTick{16, true}, frame);
    require(!std::strcmp(frame.texts[scr::t_title].chars, "variation 1 of 3") && !std::strcmp(frame.texts[scr::t_join].chars, "scene 1 > scene 1") && !std::strcmp(frame.texts[scr::t_value_2].chars, "edit"),
            "vartest's variations followed");
    page.close(now(), PageClose::back);
    m = st.save(saved, sizeof saved);
    require(saved[4 + 1] == 2, "pinned to entry 3 in the bytes");
    // The first scene 1 deleted: no scene 1 > scene 1 join. Opening elsewhere (song start) keeps them, unplaced.
    song_is({1, 0, 1});
    SongCursor start = {0, 0, 3};
    require(page.open(now(), start, actions), "opens at the start");
    page.close(now(), PageClose::back);  // the new break there: kept
    const opxy::u32 k = st.save(again, sizeof again);
    require(k > m && again[3] == 4 && (again[4 + 6] & 2) && again[4 + 1] == 2, "kept, unplaced, where they were");
    // Back as it was: placed again.
    song_is({1, 0, 0, 1});
    require(page.open(now(), moved, actions), "opens");
    vx::page_draw(PageTick{16, true}, frame);
    require(!std::strcmp(frame.texts[scr::t_value_2].chars, "edit"), "placed again");
    page.close(now(), PageClose::back);
    // The song edited again without the page opening (scene 3 inserted at the front): played,
    // vartest's second variation (track 3's filter closing in bars 3-4 after its join) would
    // now fall in the first scene 1, whose own scene matches; the scene the song came from
    // (scene 2, not scene 1) keeps it silent there. Opening the page re-pins it.
    song_is({2, 1, 0, 0, 1});
    entry = 0;
    clock_.play();
    while (entry != 2) pass();  // into the first scene 1 (the variation's old entry index)
    while (clock_.position < 62 * 480) pass();
    require(values.dsp[2][track_word::cutoff] == values.gui[2][track_word::cutoff], "came from scene 2: silent");
    while (entry != 3) pass();
    while (clock_.position < 62 * 480) pass();
    require(values.dsp[2][track_word::cutoff] == values.gui[2][track_word::cutoff], "not yet re-pinned: nothing here either");
    clock_.stop();
    values.pass(vx::hooks, now());
    SongCursor later = {0, 3, 5};
    require(page.open(now(), later, actions), "opens: re-pins");
    page.close(now(), PageClose::back);
    entry = 0;
    clock_.play();
    while (entry != 3) pass();
    while (clock_.position < 62 * 480) pass();
    require(values.dsp[2][track_word::cutoff] < 0x10000000, "re-pinned: heard at its join again");
    clock_.stop();
    values.pass(vx::hooks, now());
    // Build 025: loop off. From the song page's cursor at the song's start (join 0) a new
    // variation starts at bar 1 and can't move before it; closed after - and + took the page
    // back there, the cursor returns to the start (join 0), not the end.
    {
        loops = false;
        song_is({0, 1});
        const opxy::u32 kept = st.save(saved, sizeof saved);
        st.load(saved, 0);  // none: a new one at the start
        SongCursor start = {0, 0, 2};
        require(page.open(now(), start, actions), "opens on the song's start");
        vx::page_draw(PageTick{16, true}, frame);
        require(vx::page_model.draft().offset == 0 && !std::strcmp(frame.texts[::variations::screen::t_join].chars, "start > scene 1"),
                "loop off: a new one from bar 1, only the start named");
        page.input(now(), turn(0, -1), actions);
        require(vx::page_model.draft().offset == 0, "build 032: not back past bar 1 (031 let it reach the song's end)");
        page.close(now(), PageClose::back);
        SongCursor end = {0, 2, 2};
        st.load(saved, 0);
        require(page.open(now(), end, actions), "opens on the song's end");
        vx::page_draw(PageTick{16, true}, frame);
        require(vx::page_model.draft().offset == -32 && !std::strcmp(frame.texts[::variations::screen::t_join].chars, "scene 2 > end"),
                "loop off at the end: ending on it, only the end named");
        page.input(now(), turn(0, 1), actions);
        require(vx::page_model.draft().offset == -32, "build 032: not on past the song's end");
        page.close(now(), PageClose::back);
        st.load(saved, kept);
        loops = true;
    }
    std::puts("variations page through the extension: ok");
    return 0;
}
