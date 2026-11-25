// Host test, build 049 (2026-11-23): the variation page's timeline (mockups/variation-timeline.html): the plot
// shows the song around the join, zoomed to the window (the smallest of 32 ... 2048 steps holding twice its length,
// capped by the song the window may occupy), the view still while the window moves inside it and following at an
// edge, easing to a new target; the gesture's own curve at the window's place; the grid, the variation's join, the
// other section boundaries and the playhead; the scene names along the top and the bar numbers underneath.
// (The shape page is tests/points.cpp's, unchanged.)
#include "../page.hpp"
#include "sketch.hpp"
#include "texts.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace variations;
using opxy::extensions::PageInput;
using opxy::extensions::PageTick;
namespace draw = opxy::draw;
namespace keys = opxy::extensions::keys;
using screen::unit;

static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
// Fake metrics (as tests/metrics.cpp): a glyph is (size + its code's low bits) / 2 pixels. Off: the estimate.
static i32 fake(i32 size, const char* text) {
    i32 w = 0;
    for (const char* c = text; *c; ++c) w += (size + (*c & 7)) / 2;
    return w;
}

static Store store;
static Heard heard;
static VariationPage page;
static PageFrame frame;
static bool no_names(u32, u32, char* text, u32 capacity) {
    if (capacity) text[0] = 0;
    return false;
}
static const Names names = {no_names};
static PageInput turn(u32 encoder, i32 step, bool shift = false) {
    PageInput in = {};
    in.kind = PageInput::turn, in.encoder = encoder, in.step = step, in.shift = shift;
    return in;
}
static PageInput press(u32 key) {
    PageInput in = {};
    in.kind = PageInput::press, in.key = key;
    return in;
}
static void give(const PageInput& in) { require(!page.input(store, in, &names), "the page never closes itself"); }
static bool visible(const draw::Node& n) { return n.x != draw::hidden_x * unit; }
// A song of `count` entries of `steps` steps (scenes 0, 1, 2 ...), the page on the join into entry `entry`.
static SongShape shape;
static Around around(u32 count, u32 steps, u32 entry) {
    shape.entries = count;
    for (u32 i = 0; i < count; ++i) shape.scene[i] = static_cast<u8>(i % 90), shape.steps[i] = steps;
    Around a = {};
    a.song = true, a.song_index = 0, a.entries = count, a.entry = entry, a.shape = &shape;
    a.scene_before = shape.scene[(entry + count - 1) % count], a.scene_after = shape.scene[entry];
    a.steps_before = steps, a.steps_after = steps;
    return a;
}
static void open(const Around& a) {
    store = Store();
    page = VariationPage();
    for (u32 i = 0; i < kScenes; ++i) heard.scene[i] = 0x05;  // tracks 1 and 3 heard in every scene
    page.open(store, a, heard, &names);
    require(page.showing(), "opens");
}
// Draws a frame: valid, on screen, texts within their rooms, every shown node and path in reach.
static void draw_frame(i32 elapsed = 16, bool reset = false) {
    page.draw(PageTick{elapsed, reset}, frame);
    require(draw::valid(frame), "a valid frame");
    for (u32 i = 0; i < screen::nodes; ++i) {
        const draw::Node& n = frame.nodes[i];
        if (visible(n)) require(n.x >= 0 && n.x <= 480 * unit && n.y >= 0 && n.y <= 220 * unit, "shown nodes on screen");
    }
    texts_check::texts_ok(frame, false);
}
// Set the window's length with Shift + encoder 2 (the fixed lengths 8 ... 1024 in order; the new variation starts at 32).
static void set_length(u32 length) {
    for (u32 guard = 0; guard < 40 && page.draft().length != length; ++guard)
        give(turn(1, page.draft().length < length ? 1 : -1, true));
    if (page.draft().length != length) std::fprintf(stderr, "wanted %u, got %u (offset %d)\n", length, page.draft().length, page.draft().offset);
    require(page.draft().length == length, "the length chosen");
}
// The timeline's x (Q10) of a step in the view [from, to).
static i32 tl_x(i32 step, i32 from, i32 to) { return screen::plot_x0 * unit + (step - from) * 432 * unit / (to - from); }
static bool near_x(i32 a, i32 b) { return a - b <= unit / 4 && b - a <= unit / 4; }
// The lane shown's curve ends, x Q10: its ribbon's first top point and last (across its two paths when it is
// wide: build 050, n_shape then n_shape_b).
static void curve_ends(i32* first, i32* last) {
    const draw::Node &a = frame.nodes[screen::n_shape], &b = frame.nodes[screen::n_shape_b];
    const u32 half = screen::points / 2;
    if (visible(a) && visible(b)) {
        *first = a.x + frame.paths[screen::n_shape].points[0].x * 2;
        *last = b.x + frame.paths[screen::n_shape_b].points[half - 1].x * 2;
    } else {
        require(visible(a), "the lane's curve shown");
        *first = a.x + frame.paths[screen::n_shape].points[0].x * 2;
        *last = a.x + frame.paths[screen::n_shape].points[half - 1].x * 2;
    }
}
static i32 span_of() { return page.view_to() - page.view_from(); }

static void spans() {
    // A song of 40 entries of 4 bars, the page in the middle: room all round.
    const u32 lengths[] = {8, 16, 32, 64, 128, 256, 512, 1024};
    const i32 want[] = {32, 32, 64, 128, 256, 512, 1024, 2048};
    for (u32 k = 0; k < 8; ++k) {
        open(around(40, 64, 20));
        set_length(lengths[k]);
        draw_frame(16, true);
        char why[80];
        std::snprintf(why, sizeof why, "the span for %u steps is %d, not %d", lengths[k], want[k], span_of());
        require(span_of() == want[k], why);
        // The window within the view, and the span cap: 2 beats and 1 bar share 32 steps.
        require(page.view_from() <= page.draft().offset && page.draft().offset + static_cast<i32>(lengths[k]) <= page.view_to(), "the window in view");
    }
    // The cap: a song of two entries of 32 steps has 64 (a window up to 32 steps, its start in the section after, ...).
    open(around(2, 32, 1));
    draw_frame(16, true);
    require(span_of() == 64 && page.view_from() == -32 && page.view_to() == 32, "the song's extent caps the span (and holds the view in it)");
    // A longer window than the song's room: the span is the extent. 8 bars on a song of 4 bars either side of the join.
    // Build 050: bar lengths only as long as the song holds before the window's end: the join into entry 3 (index 2).
    open(around(3, 64, 2));
    set_length(128);
    draw_frame(16, true);
    {
        const i32 length = static_cast<i32>(page.draft().length);
        require(span_of() == 192 && span_of() < 2 * length, "a span under twice the window: the song's room (-128 .. 64)");
        require(page.view_from() == -128 && page.view_to() == 64, "the whole extent");
    }
    // Build 069 (with photographs of song 1: four entries, scenes 1, 2, 1, 2). The window is on the join into
    // entry 1, so the song's end is 192 steps away; moved as far right as it goes, it ends at 188 and the view used
    // to stop at 191 - `start_cap` holds the start one step inside its own section, and the extent was built from
    // that. The end line at 192 fell one step outside the view and so was never drawn, which is why "end" was
    // missing exactly when he had moved as far that way as he could.
    {
        open(around(4, 64, 1));
        for (u32 i = 0; i < 4 && page.draft().offset < 0; ++i) give(turn(0, 1));  // onto the join first
        require(page.draft().offset == 0, "069: on the join");
        set_length(128);  // "until" the join two on: the scene ends keep the start
        for (u32 i = 0; i < 40; ++i) give(turn(0, 1));
        draw_frame(16, true);
        const i32 song_end = 3 * 64;  // entries 1, 2 and 3 from this join
        require(page.draft().offset + static_cast<i32>(page.draft().length) <= song_end, "069: the window stays inside the song");
        require(page.view_to() >= song_end, "069: the song's end is inside the view, however far right the window is");
        const char *a = frame.texts[screen::t_label_a].chars, *b = frame.texts[screen::t_label_b].chars;
        if (std::strcmp(a, "end") != 0 && std::strcmp(b, "end") != 0) {
            std::fprintf(stderr, "offset %d, view %d..%d, labels \"%s\" \"%s\"\n",
                         page.draft().offset, page.view_from(), page.view_to(), a, b);
            require(false, "069: so the song's end is named");
        }
    }
    // Build 072 (a 2-beat window on a bar snap moved to the song's end: "the last bar is only appearing as half
    // the length it should be"). The grid loop tested `st * view_q < drawn_to_` strictly, so the line resting exactly
    // on the view's right edge was never drawn and the final cell was left unbounded. The mirror of build 066, which
    // made the same fix for a boundary on the left edge.
    {
        open(around(4, 64, 1));
        for (u32 i = 0; i < 10 && page.draft().length != 8; ++i) give(turn(1, -1, true));
        require(page.draft().length == 8, "072: a two-beat window");
        for (u32 i = 0; i < 40; ++i) give(turn(0, 1));
        draw_frame(16, true);
        i32 last = -1;
        for (u32 i = 0; i < screen::grid_count; ++i)
            if (frame.nodes[screen::n_grid + i].x != opxy::draw::hidden_x * unit && frame.nodes[screen::n_grid + i].x > last)
                last = frame.nodes[screen::n_grid + i].x;
        require(last == screen::plot_x1 * unit, "072: the grid reaches the view's right edge, so the last cell is whole");
    }
    // Build 073 (who located it by experiment: "changing the variation to a bar in length restores the end ...
    // but if I move the variation so it scrolls to the left and then come back, it's gone back to it's buggy state").
    // A length change alters the span and `aim_view` places the view anew; a move keeps the old `from` and only
    // nudges it enough to hold the window, `from = end - span`, which rests the view's right edge exactly on the
    // window's end. The rest of the bar and the song's end line then fall outside the drawn view. Nothing pulled it
    // back. Without the fix this reads view 216..248 with the second label empty.
    {
        open(around(4, 64, 0));
        for (u32 i = 0; i < 10 && page.draft().length != 8; ++i) give(turn(1, -1, true));
        require(page.draft().length == 8, "073: a two-beat window");
        for (u32 i = 0; i < 40; ++i) give(turn(0, 1));
        draw_frame(200, true);
        const i32 was_from = page.view_from(), was_to = page.view_to();
        for (u32 i = 0; i < 6; ++i) give(turn(0, -1));   // scroll away
        for (u32 i = 0; i < 8; ++i) draw_frame(200, false);
        for (u32 i = 0; i < 40; ++i) give(turn(0, 1));   // and back to the end
        for (u32 i = 0; i < 8; ++i) draw_frame(200, false);
        require(page.view_from() == was_from && page.view_to() == was_to,
                "073: coming back to the end gives the same view as arriving there");
        require(page.view_to() == 4 * 64, "073: whose right edge is the song's end, not the window's");
        const char *a = frame.texts[screen::t_label_a].chars, *b = frame.texts[screen::t_label_b].chars;
        require(!std::strcmp(a, "end") || !std::strcmp(b, "end"), "073: so the song's end is still named");
    }
    // A tiny song, two sections of 8 steps, with a 32 step window: longer than the whole song, so it cannot be
    // contained whichever way it sits. Build 066 ("the left side of the screen isn't anchored to the start"):
    // it now begins on the song's start (-8) and overhangs the end, where it used to begin 24 steps before the song
    // existed. Those leading steps never played, so anchoring to the start is the better of two degenerate fits.
    open(around(2, 8, 1));
    draw_frame(16, true);
    require(page.draft().offset == -8, "a tiny song: a new window starts on the song's start, not before it");
    require(span_of() == 32 && page.view_from() == -8 && page.view_to() == 24 && page.view_from() <= page.draft().offset &&
                page.draft().offset + static_cast<i32>(page.draft().length) <= page.view_to(),
            "a tiny song: the view is the extent, the window in it");
}

static void placing() {
    // Opens centred on the join: the new window (-32 .. 0) has its middle 16 steps from it, within half the 64 step span.
    open(around(40, 64, 20));
    draw_frame(16, true);
    require(page.view_from() == -32 && page.view_to() == 32, "centred on the join when the window's middle is within half the span of it");
    // The span changes (a longer window): the view is placed anew. Slide right eight bars. Build 070: the
    // start is no longer held inside the join's own section, so eight presses simply take it eight bars along, to
    // 96 - where 047's cap used to stop it at 63 and 068 at the last whole bar before that. The view follows.
    for (u32 i = 0; i < 8; ++i) give(turn(0, 1));
    draw_frame(16, false);
    require(page.draft().offset == 96 && page.view_from() == 64 && page.view_to() == 128, "slid eight bars on, the view following");
    // 64 steps, keeping its end (128), so the start moves to 64. Build 070: the window sits eight bars along rather
    // than being held near the join, so the view is placed on the window, 32 .. 160, not centred on the join.
    set_length(64);
    draw_frame(16, false);
    require(span_of() == 128 && page.draft().offset == 64 && page.view_from() == 32 && page.view_to() == 160, "placed anew: on the window");
    // Shorter again: 8 steps (the lengths between include scene ends, which keep the start; it ends at offset 56): span 32,
    // the window's middle well past half of it from the join, so the view is centred on the window and snapped to 4.
    // Build 069 ("end isn't there, despite me moving as far to the right as I can"): it is no longer held by
    // "the song's limit" of 63 + 8, which was the furthest the *window* may sit, not where the song ends. This song
    // runs 40 entries and carries on well past that, and the extent takes the song's own end now, so nothing holds
    // the view there.
    set_length(8);
    draw_frame(16, false);
    {
        const i32 mid = page.draft().offset + 4, near4 = ((mid - 16 + 2) / 4) * 4;
        require(span_of() == 32 && page.view_from() == near4, "placed on the window, no longer held by the window's own reach");
    }
    // And a window mid song: 16 bars on from a join in the middle of a long run, middle 60: centred on it.
    open(around(40, 64, 20));
    set_length(8);
    for (u32 i = 0; i < 3; ++i) give(turn(0, 1, false));  // -8 .. 0 on to the 16 step grid: 0? a bar a detent
    draw_frame(16, true);
    {
        const i32 mid2 = 2 * page.draft().offset + 8;  // twice the window's middle
        require(mid2 < 32 && mid2 > -32 ? page.view_from() == -16 : page.view_from() <= page.draft().offset, "the 32 step view: centred on the join when its middle is near, else on the window");
    }
}

static void still_then_follow() {
    open(around(40, 64, 20));
    draw_frame(16, true);
    const i32 from0 = page.view_from(), to0 = page.view_to();
    require(from0 == -32 && to0 == 32, "the starting view");
    i32 join_x = frame.nodes[screen::n_join].x;
    // Encoder 1 a bar a detent: the window (-32 .. 0) to -16 .. 16 and 0 .. 32: still inside the view: it stays.
    give(turn(0, 1));
    draw_frame(16, false);
    require(page.view_from() == from0 && page.view_to() == to0 && frame.nodes[screen::n_join].x == join_x, "the view stays while the window moves inside it");
    give(turn(0, 1));
    draw_frame(16, false);
    require(page.view_from() == from0 && page.draft().offset == 0 && page.draft().offset + 32 == page.view_to(), "the window touching the right edge: still");
    // One bar further: it leaves; the view shifts just enough for the window to touch that edge.
    give(turn(0, 1));
    draw_frame(16, false);
    require(page.draft().offset == 16 && page.view_to() == 48 && page.view_from() == -16, "the right edge shifted just enough: the window touches it");
    // Moving back inside: still again.
    give(turn(0, -1));
    draw_frame(16, false);
    require(page.view_from() == -16 && page.view_to() == 48, "back inside it: still");
    give(turn(0, -1));
    draw_frame(16, false);
    require(page.draft().offset == -16 && page.view_from() == -16 && page.view_to() == 48, "the window touches the left edge: still");
    give(turn(0, -1));
    draw_frame(16, false);
    require(page.draft().offset == -32 && page.view_from() == -32 && page.view_to() == 32, "past the left edge: the view followed");
    // The same far from the join, to the song's limits: the view never leaves the song (the extent).
    // Build 058: the start may go back across earlier scenes (kMaxReach = 31 entries of 64 steps from entry 20: the
    // song's first 20 entries, 1280 steps, are all there is), not just the section before (was: stops 64 before the join).
    for (u32 i = 0; i < 200; ++i) give(turn(0, -1));
    draw_frame(16, false);
    require(page.draft().offset == -1280 && page.view_from() == -1280, "the near end: the view stops at the song's limit (the song's start, 1280 before the join)");
    for (u32 i = 0; i < 80; ++i) give(turn(0, 1));
    draw_frame(16, false);
    require(page.view_to() == page.draft().offset + 32, "the far end: the window touches the right edge, which is the song's limit for it");
    require(page.view_to() - page.view_from() == 64, "the span unchanged");
}

static void easing() {
    // The first frame after open draws at the target: no ease from nothing.
    open(around(40, 64, 20));
    draw_frame(16, false);
    require(near_x(frame.nodes[screen::n_join].x, tl_x(0, page.view_from(), page.view_to())), "the first frame is at the target");
    // The page opened again elsewhere (the same page object): its first frame is at the new target, not eased from the last view.
    for (u32 i = 0; i < 3; ++i) give(turn(0, 1));
    draw_frame(16, false);
    page.close(store);
    {
        const Around other = around(40, 128, 10);
        page.open(store, other, heard, &names);
        draw_frame(16, false);
        i32 a, z;
        curve_ends(&a, &z);
        require(near_x(a, tl_x(page.draft().offset, page.view_from(), page.view_to())) && near_x(z, tl_x(page.draft().offset + static_cast<i32>(page.draft().length), page.view_from(), page.view_to())),
                "re-opened: the first frame at the target at once");
    }
    open(around(40, 64, 20));
    draw_frame(16, false);
    // A new length: the target view doubles; the drawn view approaches it each frame (k = 12 * ms / 1000), snapping at half a pixel.
    for (u32 i = 0; i < 3; ++i) give(turn(0, 1));  // the window to 16 .. 48: the view follows (-16 .. 48): the join off centre
    draw_frame(16, false);
    // Build 050: a scene end (the start kept): 16 .. 128; the view -128 .. 128, centred on the join. Build 056: the bar
    // lengths and the scene ends are in one list in length order, and a bar length moves the start (it keeps the end),
    // which changes the ends; 128 is where this climb arrives (an "until" entry, the start kept at the join's).
    set_length(128);
    const i32 start_x = frame.nodes[screen::n_join].x;
    draw_frame(16, false);  // the first frame aims the target and takes its first step
    require(page.view_to() - page.view_from() == 256, "a new target");
    const i32 target_x = tl_x(0, page.view_from(), page.view_to());
    require(!near_x(start_x, target_x) && !near_x(frame.nodes[screen::n_join].x, target_x) && frame.nodes[screen::n_join].x != start_x,
            "the first frame is on its way: not still at the old view, not yet at the new");
    i32 last = frame.nodes[screen::n_join].x, frames = 0;
    for (;; ++frames) {
        require(frames < 80, "converges");
        draw_frame(16, false);  // 16/1024 s a frame: 18 % of the way
        const i32 x = frame.nodes[screen::n_join].x;
        const i32 before = last - target_x < 0 ? target_x - last : last - target_x, after = x - target_x < 0 ? target_x - x : x - target_x;
        require(after <= before, "each frame nearer");
        // The curve clamped to the plot while the view is part way (the window may stick out of it).
        i32 a, z;
        curve_ends(&a, &z);
        require(a >= screen::plot_x0 * unit - unit / 4 && z <= screen::plot_x1 * unit + unit / 4, "the curve within the plot");
        if (near_x(x, target_x)) break;
        last = x;
    }
    require(frames > 3, "it eased over several frames, not a jump");
    // Arrived: nothing more redrawn (the revision stops being bumped): a marker survives the next frame.
    for (u32 i = 0; i < 80; ++i) {  // until the last bit has snapped
        PageFrame before = frame;
        draw_frame(16, false);
        if (!std::memcmp(&before.nodes, &frame.nodes, sizeof before.nodes)) break;
    }
    require(frame.nodes[screen::n_join].x == tl_x(0, page.view_from(), page.view_to()) || near_x(frame.nodes[screen::n_join].x, tl_x(0, page.view_from(), page.view_to())), "arrived");
    draw_frame(16, false);
    frame.nodes[screen::n_join].x = 7;
    draw_frame(16, false);
    require(frame.nodes[screen::n_join].x == 7, "settled: no more frames drawn");
    // A longer frame (88 ms: k = 1) arrives at once.
    set_length(32);
    draw_frame(90, false);
    {
        i32 a, z;
        curve_ends(&a, &z);
        require(near_x(a, tl_x(page.draft().offset, page.view_from(), page.view_to())), "k = 1: at the target in a frame");
    }
}

static void curve_and_joins() {
    // The curve's x range is the window's place in the view; the join at step 0; the later joins from the song's shape.
    open(around(40, 16, 20));  // sections of 1 bar
    give(turn(0, 1)), give(turn(0, 1));  // the window (-32 .. 0) on to 0 .. 32
    draw_frame(16, true);
    {
        const i32 from = page.view_from(), to = page.view_to();
        i32 a, z;
        curve_ends(&a, &z);
        require(near_x(a, tl_x(page.draft().offset, from, to)) && near_x(z, tl_x(page.draft().offset + 32, from, to)), "the curve from the window's start to its end");
        require(near_x(frame.nodes[screen::n_join].x, tl_x(0, from, to)) && frame.nodes[screen::n_join].y == screen::tl_cy * unit, "the join at step 0");
        // Marks at every other section boundary in view (a bar apart here), none at the join, none beyond the view.
        i32 steps_seen = 0;
        for (u32 i = 0; i < screen::mark_count; ++i) {
            if (!visible(frame.nodes[screen::n_marks + i])) continue;
            ++steps_seen;
            bool ok = false;
            for (i32 s = (from / 16 - 1) * 16; s < to + 16; s += 16)
                ok = ok || (s != 0 && near_x(frame.nodes[screen::n_marks + i].x, tl_x(s, from, to)));
            require(ok, "a mark on a section boundary");
        }
        i32 inside = 0;
        for (i32 s = (from / 16) * 16; s < to; s += 16) inside += s > from && s != 0;
        require(steps_seen == inside, "a mark on every other boundary strictly in view");
    }
}

static void playhead() {
    open(around(8, 64, 3));
    draw_frame(16, true);
    require(!visible(frame.nodes[screen::n_cursor]) && !visible(frame.nodes[screen::n_playhead]), "hidden while stopped");
    SongPosition p = {};
    p.playing = true, p.song = 0, p.entry = 3, p.next_entry = 4, p.position = 10 * 480, p.length = 64 * 480, p.scene = 3, p.scene_before = 2;
    page.follow(p);
    draw_frame(16, false);
    // The song's place 10 steps after the join, in the view -32 .. 32.
    require(visible(frame.nodes[screen::n_cursor]) && frame.nodes[screen::n_cursor].x == tl_x(10, -32, 32) / unit * unit &&
                !visible(frame.nodes[screen::n_playhead]),
            "the playhead at the song's position");
    p.position = 50 * 480;  // past the view
    page.follow(p);
    draw_frame(16, false);
    require(!visible(frame.nodes[screen::n_cursor]), "outside the shown view: hidden");
}

// The scene names: where each section starts in view + 4, in the slots in order; left out when the section shows
// under 44 px; "start, scene 1" and "scene N, end" on the song's start.
static void names_row() {
    open(around(40, 64, 20));
    draw_frame(16, true);
    // The view -32 .. 32: the section before (-64 .. 0) shows 32 steps (216 px), the one after 32.
    require(!std::strcmp(frame.texts[screen::t_label_a].chars, "scene 20") && !std::strcmp(frame.texts[screen::t_label_b].chars, "scene 21"),
            "the section before and the one after");
    require(near_x(frame.texts[screen::t_label_a].x, screen::plot_x0 * unit + screen::name_inset * unit) && near_x(frame.texts[screen::t_label_b].x, tl_x(0, -32, 32) + screen::name_inset * unit) &&
                frame.texts[screen::t_label_a].y == (screen::tl_y0 - 8) * unit && frame.texts[screen::t_label_b].y == (screen::tl_y0 - 8) * unit,  // build 050: above the plot
            "4 px right of each section's start in view, on the plot's top");
    // A long window: the sections after it in turn (build 052: two slots; the third, t_target, is gone).
    open(around(40, 64, 20));
    set_length(256);
    draw_frame(16, true);
    // Build 049: two names at most (a 12 px third slot drew one larger than the others; build 052 took it away).
    require(!std::strcmp(frame.texts[screen::t_label_a].chars, "scene 20") && !std::strcmp(frame.texts[screen::t_label_b].chars, "scene 21"),
            "a longer view: the first two sections named, no third");
    const i32 a = frame.texts[screen::t_label_a].x, b = frame.texts[screen::t_label_b].x;
    require(a < b, "in order along the top");
    // Cramped: sections of 16 steps in a 512 step view are 14 px: no names at all.
    open(around(80, 16, 40));
    set_length(256);
    draw_frame(16, true);
    require(span_of() == 512 && !frame.texts[screen::t_label_a].chars[0] && !frame.texts[screen::t_label_b].chars[0],
            "sections under 44 px: no names");
    // Build 051: "end" at the song's end, left of its line; beside it the scene the window covers.
    open(around(3, 64, 1));
    give(turn(0, 2));  // the window onto the join: 0 .. 32
    for (u32 i = 0; i < 8 && page.draft().length != 128; ++i) give(turn(1, 1, true));
    require(page.draft().offset == 0 && page.draft().length == 128, "until end: 0 .. 128");
    draw_frame(16, true);
    {
        const char *a = frame.texts[screen::t_label_a].chars, *b = frame.texts[screen::t_label_b].chars;
        // Build 061 ("you don't see start on the timeline when moving the variation, which is confusing"): the
        // whole song is in view here - its start is 64 steps back, its end 128 on, and the view runs -128 to 128 -
        // so both lines are named and neither slot is left for a scene name. Until 060 the start line sat unlabelled
        // while the end line read "end", which is the asymmetry reported.
        require(!std::strcmp(a, "start") && !std::strcmp(b, "end"), "both ends of the song named");
        require(page.view_from() <= -64 && page.view_to() >= 128, "and both really are in view");
        const i32 end_x = tl_x(128, page.view_from(), page.view_to());
        require(frame.texts[screen::t_label_b].x < end_x && frame.texts[screen::t_label_b].x > end_x - 40 * unit, "end just left of the song's end");
        bool line = false;
        for (u32 i = 0; i < screen::mark_count; ++i)
            line = line || (visible(frame.nodes[screen::n_marks + i]) && near_x(frame.nodes[screen::n_marks + i].x, end_x));
        require(line, "a scene line at the song's end");
    }
    // Build 061: with the song's start out of view, "end" keeps a scene name beside it, as it has since build 051.
    open(around(10, 64, 8));
    give(turn(0, 2));
    for (u32 i = 0; i < 8 && page.draft().length != 128; ++i) give(turn(1, 1, true));
    draw_frame(16, true);
    {
        const char *a = frame.texts[screen::t_label_a].chars, *b = frame.texts[screen::t_label_b].chars;
        require(page.view_from() > -512, "the song's start is out of view");
        require(!std::strcmp(b, "end") && a[0] && std::strcmp(a, "start") != 0, "a scene name, then end");
    }
    // Whatever shows never overlaps and stays inside the plot, over many views.
    for (u32 steps = 8; steps <= 128; steps *= 2)
        for (u32 length = 8; length <= 1024; length *= 2) {
            if (length > steps * 30) continue;  // build 050: longer than the song holds before the join
            open(around(70, steps, 30));
            set_length(length);
            draw_frame(16, true);
            const u32 slots[2] = {screen::t_label_a, screen::t_label_b};
            i32 end = 0;
            for (u32 k = 0; k < 2; ++k) {
                const draw::Text& t = frame.texts[slots[k]];
                if (!t.chars[0]) continue;
                const i32 w = words::width(t.chars, screen::label_size);
                require(t.x >= end && t.x + w <= screen::plot_x1 * unit && t.y == (screen::tl_y0 - 8) * unit, "names apart, within the plot's width, above it (build 050)");
                end = t.x + w;
            }
        }
    // The song's start: "scene 2, end" for the section before, "start, scene 1" for the one after.
    {
        static SongShape two = {2, {0, 1}, {64, 64}};
        Around s = {};
        s.song = true, s.entries = 2, s.entry = 0, s.scene_before = 1, s.scene_after = 0, s.steps_before = 64, s.steps_after = 64, s.shape = &two;
        open(s);
        draw_frame(16, true);
        require(!std::strcmp(frame.texts[screen::t_label_a].chars, "scene 2, end") && !std::strcmp(frame.texts[screen::t_label_b].chars, "start, scene 1"),
                "the song's start forms");
    }
}

// Build 049 (option a): the bar the window starts in, its left edge 2 px right of the window's start
// (t_number_a). Bars from the join: 1 the first after it, -1 the last before it. Build 054: the window's end bar lost its
// slot (the second M-key mark); build 056: it is back in t_number_a, as a range "5-8" ("-2>1" with a negative end;
// the start's bar alone when the first and last steps share one). Returns how many show.
static i32 bar_of(i32 step) { return step >= 0 ? step / 16 + 1 : -((-step - 1) / 16 + 1); }
static u32 check_numbers(const char* why) {
    const draw::Text& s = frame.texts[screen::t_number_a];
    const i32 from = page.view_from(), to = page.view_to();
    const i32 first = page.draft().offset, last = first + static_cast<i32>(page.draft().length) - 1;
    const i32 a = tl_x(first, from, to), b = tl_x(last + 1, from, to);
    require(s.y == (screen::tl_y1 + 13) * unit, "the start's number 13 px under the plot");
    {
        char want[16];
        const i32 f = bar_of(first), l = bar_of(last);
        if (f == l) std::snprintf(want, sizeof want, "%d", f);
        else std::snprintf(want, sizeof want, f < 0 || l < 0 ? "%d>%d" : "%d-%d", f, l);
        if (std::strcmp(s.chars, want)) std::fprintf(stderr, "%s: shows \"%s\", wants \"%s\"\n", why, s.chars, want);
        require(!std::strcmp(s.chars, want), "the window's bar range: its first and last bars");
    }
    const i32 sw = words::width(s.chars, screen::label_size);
    if (!near_x(s.x, a + screen::number_inset * unit) && s.x + sw != screen::plot_x1 * unit) {
        std::fprintf(stderr, "%s: start \"%s\" at %d, window start %d (1/1024 px)\n", why, s.chars, s.x, a);
        require(false, "the number number_inset right of the window's start");
    }
    (void)b;
    (void)last;
    return 1;
}

static void numbers() {
    for (u32 measured = 0; measured < 2; ++measured) {
        draw::host_text_measure() = measured ? fake : nullptr;
        // Windows from 2 beats to 64 bars.
        for (u32 length = 8; length <= 1024; length *= 2) {
            open(around(70, 64, 30));
            set_length(length);
            draw_frame(16, true);
            char why[40];
            std::snprintf(why, sizeof why, "length %u%s", length, measured ? " measured" : "");
            require(check_numbers(why) == 1, "build 056: the bar range");
        }
        // A new variation: 2 bars ending on the join: -2 and -1.
        open(around(70, 64, 30));
        draw_frame(16, true);
        require(!std::strcmp(frame.texts[screen::t_number_a].chars, "-2>-1"), "a new break: bars -2 to -1 (build 056: \">\" for a negative end)");
        // Build 049: the band: strips 16 px wide from the window's start to its end, behind; none past it.
        {
            const i32 from = page.view_from(), to = page.view_to();
            const i32 a = tl_x(page.draft().offset, from, to), b = tl_x(page.draft().offset + static_cast<i32>(page.draft().length), from, to);
            i32 lo = 1 << 30, hi = -(1 << 30);
            for (u32 k = 0; k < screen::band_count; ++k) {
                const draw::Node& n = frame.nodes[screen::n_band + k];
                if (!visible(n)) continue;
                require(n.shade == screen::s_band && n.y == screen::tl_cy * unit, "a band strip: the band's grey, on the plot");
                if (n.x - 8 * unit < lo) lo = n.x - 8 * unit;
                if (n.x + 8 * unit > hi) hi = n.x + 8 * unit;
            }
            // Build 056: the band stops band_inset px inside the window's ends, clear of a marker on either.
            require(near_x(lo, a + screen::band_inset * unit) && near_x(hi, b - screen::band_inset * unit), "the band spans the window less its inset");
            page.input(store, press(keys::m1 + 2), &names);  // the shape page: no band
            draw_frame(16, true);
            for (u32 k = 0; k < screen::band_count; ++k) require(!visible(frame.nodes[screen::n_band + k]), "no band on the shape page");
            page.input(store, press(keys::m1 + 2), &names);
            draw_frame(16, true);
        }
        // Moved on to the join: 1 and 2.
        for (u32 i = 0; i < 2; ++i) give(turn(0, 1));
        draw_frame(100, false);
        check_numbers("on the join");
        // Build 056 (the join line obscured the first digit): a window starting exactly on the join has its number's
        // left edge at least 2 px right of the join line's right edge (the line is 2 px wide: its half is 1 px).
        {
            require(page.draft().offset == 0 && visible(frame.nodes[screen::n_join]), "the window starts on the join, which is in view");
            const draw::Text& t = frame.texts[screen::t_number_a];
            require(t.x >= frame.nodes[screen::n_join].x + 1 * unit + 2 * unit, "build 056: the number 2 px clear of the join line's right edge");
        }
        require(!std::strcmp(frame.texts[screen::t_number_a].chars, "1-2"), "on the join: bars 1 to 2 (build 056)");
    }
    draw::host_text_measure() = nullptr;
}

// Build 056 ("if I move my 2 bar variation to the last scene, it overlaps the scene marker on the grid"): the band
// stops band_inset px inside the window's ends, so a scene marker or the song's end line on one stays whole: its drawn
// x at least 1 px clear of the band's edge. A song of 2-bar scenes, a 2-bar window, once ending on a scene boundary
// (the next scene's line) and once on the song's end.
static void band_and_markers() {
    // Case 0: a 4-bar window across the join, both its edges on scene lines in view (the section before's start and the
    // next join on); case 1: a 2-bar window ending on the song's end (the end line, at the view's edge).
    for (u32 end_of_song = 0; end_of_song < 2; ++end_of_song) {
        open(around(4, 32, end_of_song ? 3 : 2));  // 4 scenes of 2 bars: the join into the third, or the last (the song ends 32 on)
        if (!end_of_song) {
            set_length(64);
            for (u32 i = 0; i < 40 && page.draft().offset < -32; ++i) give(turn(0, 1));
            require(page.draft().length == 64 && page.draft().offset == -32, "a 4-bar window from the section before's start");
        } else {
            for (u32 i = 0; i < 40 && page.draft().offset < 0; ++i) give(turn(0, 1));
            require(page.draft().length == 32 && page.draft().offset == 0, "a 2-bar window ending on the song's end");
        }
        draw_frame(16, true);
        for (u32 i = 0; i < 80; ++i) draw_frame(16, false);  // the view settles
        const i32 from = page.view_from(), to = page.view_to();
        const i32 left = tl_x(page.draft().offset, from, to), right = tl_x(page.draft().offset + static_cast<i32>(page.draft().length), from, to);
        i32 hi = -(1 << 30), lo = 1 << 30;
        for (u32 k = 0; k < screen::band_count; ++k) {
            const draw::Node& n = frame.nodes[screen::n_band + k];
            if (!visible(n)) continue;
            if (n.x + 8 * unit > hi) hi = n.x + 8 * unit;
            if (n.x - 8 * unit < lo) lo = n.x - 8 * unit;
        }
        u32 found = 0;
        for (u32 m = 0; m < screen::mark_count; ++m) {
            const draw::Node& n = frame.nodes[screen::n_marks + m];
            if (!visible(n)) continue;
            if (!end_of_song && near_x(n.x, left)) ++found, require(lo >= n.x + unit, "build 056: the band's left edge at least 1 px clear of the marker's drawn x");
            if (near_x(n.x, right)) ++found, require(hi <= n.x - unit, "build 056: the band's right edge at least 1 px clear of the marker's drawn x");
        }
        if (found != (end_of_song ? 1u : 2u)) std::fprintf(stderr, "eos %u found %u lo %d hi %d off %d len %u view %d..%d\n", end_of_song, found, lo, hi, page.draft().offset, page.draft().length, from, to);
        require(found == (end_of_song ? 1u : 2u) && hi > lo, "the markers on the window's edges are drawn, and so is the band");
    }
}

// Build 056 ("I'm no longer seeing the option to resize a variation more than 2 bars"; "I'd expect it to expand
// left to fill the clip even if you're at the end, but right now it blocks the change"): a song of 2-bar scenes, the
// page on the last scene's join, a 2-bar window at offset 0: the lengths step up past the first scene end to 4 bars and
// beyond, and the list holds no fixed length equal to a scene end (the "until" entry says it).
static void lengths_above_first_end() {
    open(around(4, 32, 3));  // the last scene's join: 32 steps after it, the song's end
    for (u32 i = 0; i < 40 && page.draft().offset < 0; ++i) give(turn(0, 1));
    require(page.draft().offset == 0 && page.draft().length == 32, "a 2-bar window at offset 0 on the last scene's join");
    u32 seen[24], n = 0;
    seen[n++] = page.draft().length;
    for (u32 i = 0; i < 20; ++i) {
        const u32 was = page.draft().length;
        give(turn(1, 1, true));
        if (page.draft().length == was) break;
        seen[n++] = page.draft().length;
    }
    u32 most = 0;
    for (u32 i = 0; i < n; ++i) most = seen[i] > most ? seen[i] : most;
    require(most >= 64, "stepping the length up reaches at least 4 bars (64 steps)");
    require(page.draft().length != 32 && n > 2, "the length stepped on, not blocked at the first scene end");
    // The list is in length order and holds each length once (a fixed length equal to a scene end is left out: the
    // "until" entry says it), so stepping up only ever lengthens the window.
    for (u32 i = 1; i < n; ++i) require(seen[i] > seen[i - 1], "each step up lengthens the window: no length listed twice");
}

// A curve wider than a path reaches (a window over most of the view): the lane shown across both paths, as the
// shape page's line; the other lane left out.
static void wide_curve() {
    // Build 050: bar lengths only as long as the song holds before the window's end: the join into entry 3 (index 2).
    open(around(3, 64, 2));
    set_length(128);
    give(turn(1, 1));  // a gesture
    draw_frame(16, true);
    // Build 050: each lane on two paths: the lane shown bright, the other lane (it follows the gesture: a's) still shown, dim.
    const draw::Node &a = frame.nodes[screen::n_shape], &b = frame.nodes[screen::n_shape_b];
    require(visible(a) && visible(b) && a.shade == b.shade && a.shade == screen::s_bright, "both halves the lane shown's");
    i32 first, last;
    curve_ends(&first, &last);
    require(near_x(first, tl_x(page.draft().offset, page.view_from(), page.view_to())) &&
                near_x(last, tl_x(page.draft().offset + 128, page.view_from(), page.view_to())),
            "across the window's place");
    require(last - first > 250 * unit, "wider than one path reaches");
    // The halves meet at the middle.
    const i32 mid_a = a.x + frame.paths[screen::n_shape].points[screen::points / 2 - 1].x * 2, mid_b = b.x + frame.paths[screen::n_shape_b].points[0].x * 2;
    require(near_x(mid_a, mid_b), "the two halves meet");
    // Build 050 ("we seem to have lost the second lane"): to lane b (encoder 1's tap; b empty): lane a, now
    // the other lane, still drawn across the window on its two paths, dim behind.
    give(press(keys::m1));
    draw_frame(16, true);
    require(page.lane_index() == 1, "lane b shown");
    // Build 060: the other lane is sketched from nodes now (31 pieces, each laid from one sample to the next), dim, from the
    // window's first end to its last across the view, and the paths it had are hidden.
    require(!visible(frame.nodes[screen::n_other]) && !visible(frame.nodes[screen::n_other_b]), "build 060: not on paths");
    require(sketch_check::pieces_shown(frame, 1, 0) == screen::sketch_pieces && sketch_check::pieces_shade(frame, 1, 0, screen::s_other),
            "the other lane's sketch, dim");
    const draw::Node &p0 = frame.nodes[screen::n_sketch], &p1 = frame.nodes[screen::n_sketch + screen::sketch_pieces - 1];
    const i32 o_first = p0.x, o_last = p1.x + screen::sketch_length * p1.scale * p1.cos / 1024;
    require(near_x(o_first, first) && (o_last - last < unit / 2 && last - o_last < unit / 2), "across the same place as before");
    // The pieces run end to end: each starts where the one before reached.
    for (u32 k = 1; k < screen::sketch_pieces; ++k) {
        const draw::Node &q = frame.nodes[screen::n_sketch + k - 1], &n = frame.nodes[screen::n_sketch + k];
        const i32 ex = q.x + screen::sketch_length * q.scale * q.cos / 1024;
        require(n.x - ex < unit / 2 && ex - n.x < unit / 2, "the sketch's pieces meet");
    }
}

// Frames through random inputs and frame times (mid-ease views, windows part way out of the view or wider than a
// path reaches, songs of every shape): every frame valid, on screen, its texts in their rooms, its numbers aligned.
static void fuzz() {
    u32 seed = 12345;
    auto next = [&](u32 n) { return (seed = seed * 1664525u + 1013904223u) / 65536 % n; };
    static SongShape two = {2, {0, 1}, {64, 96}};
    for (u32 measured = 0; measured < 2; ++measured) {
        draw::host_text_measure() = measured ? fake : nullptr;
        for (u32 song = 0; song < 5; ++song) {
            Around a = song == 0 ? around(16, 64, 6) : song == 1 ? around(5, 48, 1) : song == 2 ? around(16, 16, 9) : song == 3 ? around(3, 400, 0) : Around();
            if (song == 4) {
                a.song = true, a.entries = 2, a.entry = 0, a.scene_before = 1, a.scene_after = 0, a.steps_before = 96, a.steps_after = 64, a.shape = &two;
                a.stops = true;  // the song's start of a song that doesn't loop
            }
            open(a);
            for (u32 step = 0; step < 700; ++step) {
                const u32 pick = next(10);
                if (pick < 4) give(turn(0, next(2) ? 1 : -1, next(5) == 0));
                else if (pick < 6) give(turn(1, next(2) ? 1 : -1, true));
                else if (pick == 6) give(turn(1, next(2) ? 1 : -1));
                else if (pick == 7) {
                    PageInput in = {};
                    in.kind = PageInput::click, in.encoder = 0, in.key = keys::encoder;
                    give(in);
                }
                const i32 elapsed = next(4) == 0 ? static_cast<i32>(next(120)) : 16;
                draw_frame(elapsed, next(25) == 0);
                for (u32 o = 0; o < screen::paths; ++o)
                    for (u32 i = 0; i < screen::points; ++i) {
                        const draw::Point& q = frame.paths[o].points[i];
                        require(q.x >= -64 * unit && q.x <= 64 * unit, "the path within a node's reach");
                    }
                if (step % 5 == 0) {  // settled (k = 1), the numbers are the target view's
                    draw_frame(100, false);
                    if (frame.texts[screen::t_number_a].chars[0]) check_numbers("fuzz");
                }
                // A curve never leaves the plot.
                if (visible(frame.nodes[screen::n_shape])) {
                    i32 first, last;
                    curve_ends(&first, &last);
                    require(first >= screen::plot_x0 * unit - unit / 4 && last <= screen::plot_x1 * unit + unit / 4 && first < last, "the curve within the plot");
                }
            }
        }
    }
    draw::host_text_measure() = nullptr;
}

int main() {
    spans();
    placing();
    still_then_follow();
    easing();
    curve_and_joins();
    playhead();
    names_row();
    numbers();
    wide_curve();
    band_and_markers();
    lengths_above_first_end();
    fuzz();
    std::puts("variations timeline: ok");
    return 0;
}
