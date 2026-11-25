// Host test: the third lane (build 060, stage B). M1 steps a, b, c, a on both pages, plain or with Shift; lane C is
// edited as the others are (a gesture, a target, tracks, mute, a hand-drawn point), each committed to the store and read
// back; all three lanes are drawn at once (the lane shown on its paths, the other two sketched from nodes, whatever the
// window's width), every frame valid and within the page's 4 paths and 256 nodes; the lane letters a, b and c, the
// shown one bright and the row centred on M1's key; and a variation with three lanes surviving the page's close and
// reopen.
#include "../page.hpp"
#include "sketch.hpp"
#include "texts.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
using namespace variations;
using opxy::extensions::PageInput;
namespace draw = opxy::draw;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static Store store;
static Heard heard;
static VariationPage page;
static PageFrame frame;
using screen::unit;

static PageInput turn(u32 encoder, i32 step, bool shift = false, u32 speed = 0) {
    PageInput in = {};
    in.kind = PageInput::turn, in.encoder = encoder, in.step = step, in.shift = shift, in.speed = speed;
    return in;
}
static PageInput press(u32 key, bool shift = false) {
    PageInput in = {};
    in.kind = PageInput::press, in.key = key, in.shift = shift;
    return in;
}
static void give(const PageInput& in) { require(!page.input(store, in), "the page never closes itself here"); }
// M3, the way into the shape page and back out (build 054; 022-053 encoder 2's tap, 021's encoder 4).
static void tap_2() { give(press(keys::m1 + 2)); }
static Around song_1() {
    Around a = {};
    a.song = true, a.song_index = 0, a.entries = 2, a.entry = 1;
    a.scene_before = 0, a.scene_after = 1;
    a.steps_before = 64, a.steps_after = 64;
    return a;
}
static bool visible(const draw::Node& n) { return n.x != draw::hidden_x * unit; }


// Lane a: a rise on the cutoff, tracks 3 and 5; lane b: a fall on track 6; lane c: a break on track 1, the volume.
static Draft example3(u32 length = 32) {
    Draft d = {};
    d.song = 0, d.entry = 1, d.length = length, d.offset = -static_cast<i32>(length);
    d.scene_before = 0, d.scene_after = 1;
    clear_lane(d.lanes[0]), clear_lane(d.lanes[1]), clear_lane(d.lanes[2]);
    DraftLane& a = d.lanes[0];
    a.gesture = rise, a.amount = 128, a.target = kCutoff, a.selection = 0x14, a.curve = default_curve(rise);
    DraftLane& b = d.lanes[1];
    b.gesture = fall, b.amount = 128, b.target = kCutoff, b.selection = 0x20, b.curve = default_curve(fall);
    DraftLane& c = d.lanes[2];
    c.gesture = brk, c.amount = 160, c.target = kVolume, c.selection = 0x01, c.curve = default_curve(brk);
    generate(d);
    return d;
}

// Lanes a and b only (lane c empty).
static Draft example(u32 length = 32) {
    Draft d = example3(length);
    clear_lane(d.lanes[2]);
    return d;
}
// A fresh store holding `d` as its one variation.
static void fresh(const Draft& d) {
    store = Store();
    require(store.put(0, d), "put");
}

// Every frame: valid, shown nodes on screen, texts in their rooms; the lane shown on its paths (variation page) and the
// others on sketch nodes, all or none of a lane's pieces, dim; never an other lane on a path; the lane letters drawn.
static void frame_ok(bool reset = true) {
    using namespace screen;
    PageTick tick = {16, reset};
    page.draw(tick, frame);
    require(draw::valid(frame), "a valid frame");
    require(nodes <= 256 && paths <= 4, "within the SDK's 4 paths and 256 nodes");
    for (u32 i = 0; i < nodes; ++i) {
        const draw::Node& n = frame.nodes[i];
        if (!visible(n)) continue;
        require(n.x >= 0 && n.x <= 480 * unit && n.y >= 0 && n.y <= 220 * unit, "shown nodes on screen");
    }
    texts_check::texts_ok(frame, page.on_points());
    const u32 shown_lane = page.lane_index();
    if (page.on_points()) {
        require(sketch_check::all_shown(frame) == 0, "the shape page draws no sketch");
        return;
    }
    require(!visible(frame.nodes[n_other]) && !visible(frame.nodes[n_other_b]), "no other lane on a path");
    for (u32 lane = 0; lane < kLanes; ++lane) {
        if (lane == shown_lane) continue;
        const DraftLane& o = page.draft().lanes[lane];
        const u32 n = sketch_check::pieces_shown(frame, shown_lane, lane);
        require(n == 0 || n == sketch_pieces, "a sketch is all its pieces or none");
        require(n == 0 || (o.gesture != kNoGesture && o.selection && o.count), "only a lane with something to draw is sketched");
        require(sketch_check::pieces_shade(frame, shown_lane, lane, o.muted ? s_dim : s_other), "the other lanes dim");
    }
}
// The lane letters: all three drawn, the lane shown bright and the others mid, a, b, c left to right.
static bool letters_show(u32 lane) {
    using namespace screen;
    const u32 all[5] = {n_ring_a, n_stem_a, n_ring_b, n_stem_b, n_letter_c};
    for (u32 n : all)
        if (!visible(frame.nodes[n])) return false;
    const bool a = lane == 0, b = lane == 1, c = lane == 2;
    return frame.nodes[n_ring_a].shade == (a ? s_ring_white : s_ring_mid) && frame.nodes[n_stem_a].shade == (a ? s_white : s_mid) &&
           frame.nodes[n_ring_b].shade == (b ? s_ring_white : s_ring_mid) && frame.nodes[n_stem_b].shade == (b ? s_white : s_mid) &&
           frame.nodes[n_letter_c].shade == (c ? s_white : s_mid) && frame.nodes[n_ring_b].x > frame.nodes[n_ring_a].x &&
           frame.nodes[n_letter_c].x > frame.nodes[n_ring_b].x;
}
// M1, plain or with Shift (released after, as the hardware does).
static void unshift() {
    PageInput up = {};
    up.kind = PageInput::release, up.key = keys::shift, up.shift = false;
    give(up);
}
static void m1(bool shift) {
    give(press(keys::m1, shift));
    if (shift) {
        PageInput up = {};
        up.kind = PageInput::release, up.key = keys::shift, up.shift = false;
        give(up);
    }
}

int main() {
    draw::host_text_measure() = texts_check::device_like;
    using namespace screen;

    // M1 cycles a, b, c, a, on both pages, plain and with Shift; showing a lane is no edit, and an empty lane c stays empty
    // (lane b, empty, takes lane a's gesture the first time it shows; lane c never does).
    for (u32 on_points = 0; on_points < 2; ++on_points) {
        for (u32 shift = 0; shift < 2; ++shift) {
            fresh(example());
            page.open(store, song_1(), heard);
            if (on_points) tap_2();
            require(page.on_points() == (on_points != 0) && page.lane_index() == 0, "opened on lane a");
            const u32 want[7] = {1, 2, 0, 1, 2, 0, 1};
            for (u32 i = 0; i < 7; ++i) {
                m1(shift != 0);
                require(page.lane_index() == want[i] && page.on_points() == (on_points != 0), "M1: a, b, c, a, b, c, a ... on both pages");
                frame_ok();
                require(letters_show(want[i]), "the shown lane's letter bright");
                require(store.count() == 1 && !page.fresh(), "switching lanes is no edit");
            }
            require(page.draft().lanes[2].gesture == kNoGesture && !page.draft().lanes[2].selection && !page.draft().lanes[2].count &&
                        store[0].lanes[2].gesture == kNoGesture,
                    "lane c, never edited, is still empty and still stored empty: it takes nothing from lane a");
            require(store[0].lanes[0].gesture == rise && store[0].lanes[1].gesture == fall, "lanes a and b as they were");
            page.close(store);
        }
    }

    // Lane c editable on the variation page: tracks, a gesture, a target, an amount, mute; each stored and read back.
    {
        fresh(example());
        page.open(store, song_1(), heard);
        m1(false), m1(false);
        require(page.lane_index() == 2, "lane c");
        const DraftLane& c = page.draft().lanes[2];
        give(press(keys::track + 3));
        require(c.selection == 0x08 && store[0].lanes[2].selection == 0x08 && !c.default_tracks, "a track on lane c, stored");
        give(turn(1, 3));
        require(c.gesture != kNoGesture && c.selection == 0x08 && c.count > 0 && store[0].lanes[2].gesture == c.gesture &&
                    store[0].lanes[2].count == c.count,
                "a gesture on lane c, on its track, stored");
        frame_ok(false);
        const u32 target = c.target;
        give(turn(2, 1));
        require(c.target != target && store[0].lanes[2].target == c.target, "lane c's target, stored");
        const u32 amount = c.amount;
        give(turn(3, 4));
        require(c.amount != amount && store[0].lanes[2].amount == c.amount, "lane c's amount, stored");
        PageLeds leds = {};
        page.leds(leds);
        require(leds.track[3] == led::on && leds.track[2] == led::off, "lane c's tracks on the keys");
        give(press(keys::m1 + 1));
        require(c.muted && store[0].lanes[2].muted && !page.draft().lanes[0].muted && !page.draft().lanes[1].muted, "lane c muted, stored; the others not");
        frame_ok(false);
        require(frame.nodes[n_shape].shade == s_dim, "its curve dim");
        // The others are untouched by all of it.
        require(store[0].lanes[0].gesture == rise && store[0].lanes[1].gesture == fall && store[0].lanes[0].selection == 0x14,
                "lanes a and b untouched");
        // A hand-drawn point, on the shape page (M3): a point at the cursor, then the line is custom/edited, stored.
        give(press(keys::m1 + 1));  // unmute
        tap_2();
        require(page.on_points() && page.lane_index() == 2, "the shape page on lane c");
        frame_ok();
        require(visible(frame.nodes[0]) && visible(frame.nodes[1]) && frame.nodes[1].shade == s_bright, "lane c's line drawn, bright, on its two paths");
        const u32 count = c.count;
        give(turn(0, 2));  // the cursor off any point
        give(turn(1, 3));  // a value: adds a point there
        require(c.edited && c.count >= count && store[0].lanes[2].edited && store[0].lanes[2].count == c.count, "a hand-drawn point on lane c, stored");
        const DraftLane before = c;
        give(press(keys::track + 5));  // a track toggles there too
        require(c.selection == 0x28 && c.edited && store[0].lanes[2].selection == 0x28 && c.count == before.count, "a track on lane c's shape page keeps the line");
        give(press(keys::m1 + 1));  // mute dims the line
        frame_ok(false);
        require(c.muted && frame.nodes[0].shade == s_dim && frame.nodes[1].shade == s_dim, "muted: lane c's line dim on the shape page");
        give(press(keys::m1 + 1));
        require(store[0].lanes[0].gesture == rise && store[0].lanes[1].gesture == fall, "lanes a and b untouched, still");
        page.close(store);
    }

    // All three drawn at once, on every window length, each lane shown in turn, on both pages, plain and with Shift held:
    // the shown lane on its paths, the other two sketched, every frame valid.
    {
        fresh(example3());
        page.open(store, song_1(), heard);
        for (u32 i = 0; i < kLengthChoices + 4; ++i) give(turn(1, -1, true));  // Shift + encoder 2: down to the shortest
        unshift();
        u32 drawn_frames = 0, wide_frames = 0;
        for (u32 k = 0; k < kLengthChoices + 2; ++k) {
            for (u32 lane = 0; lane < kLanes; ++lane) {
                require(page.lane_index() == lane, "lane in turn");
                frame_ok();
                require(letters_show(lane), "its letter bright");
                if (page.draft().length > 0) {
                    // The window is in view (its width at least curve_min px): the shown lane on its path, the others all sketched.
                    const bool drawn = visible(frame.nodes[n_shape]);
                    drawn_frames += drawn, wide_frames += visible(frame.nodes[n_shape_b]);
                    for (u32 other = 0; other < kLanes; ++other) {
                        if (other == lane) continue;
                        const u32 n = sketch_check::pieces_shown(frame, lane, other);
                        require(n == (drawn ? sketch_pieces : 0), "every other lane sketched with the shown one: none vanishes under the other");
                    }
                    require(!drawn || (visible(frame.nodes[n_shape]) && frame.nodes[n_shape].shade == s_bright), "the shown lane bright on its path");
                }
                // With Shift held the letters stand down; the sketches stay.
                give(press(keys::shift, true));
                frame_ok(false);
                require(!visible(frame.nodes[n_letter_c]) && !visible(frame.nodes[n_ring_a]), "Shift: the letters give way");
                PageInput up = {};
                up.kind = PageInput::release, up.key = keys::shift, up.shift = false;
                give(up);
                // The shape page on this lane: its line on the paths, no sketch, still valid.
                tap_2();
                frame_ok();
                require(page.on_points() && visible(frame.nodes[0]) && visible(frame.nodes[1]), "the lane's own line, both halves");
                tap_2();
                m1(false);
            }
            give(turn(1, 1, true));  // the next length up
            unshift();
        }
        require(drawn_frames > 0 && wide_frames > 0, "the sweep drew narrow and wide curves (the lane shown on one path or two)");
        page.close(store);
    }

    // The letters: three drawn, the lane shown bright, the row centred on M1's key (a's left bowl edge to c's right).
    {
        fresh(example3());
        page.open(store, song_1(), heard);
        for (u32 on_points = 0; on_points < 2; ++on_points) {
            if (on_points) tap_2();
            for (u32 lane = 0; lane < kLanes; ++lane) {
                frame_ok();
                require(letters_show(lane), "three letters, the shown one bright");
                const i32 r = ring_r10 * letters_scale / 10;
                const i32 edge_a = frame.nodes[n_ring_a].x - r, edge_c = frame.nodes[n_letter_c].x + r;
                const i32 dx = (edge_a + edge_c) / 2 - m1_x;
                require(dx >= -2 * unit && dx <= 2 * unit, "the letters centred on M1's key");
                require(frame.nodes[n_letter_c].scale == letters_scale && frame.nodes[n_letter_c].y == frame.nodes[n_ring_a].y,
                        "c at the letters' scale, on a's bowl line");
                m1(false);
            }
        }
        page.close(store);
    }

    // Three lanes survive the page's close and reopen, and a store round trip.
    {
        fresh(example3());
        page.open(store, song_1(), heard);
        m1(false), m1(false);
        give(turn(3, 3));  // lane c's amount
        const DraftLane c = page.draft().lanes[2];
        page.close(store);
        VariationPage again;
        again.open(store, song_1(), heard);
        const Draft& d = again.draft();
        require(again.stored() && d.lanes[0].gesture == rise && d.lanes[1].gesture == fall && d.lanes[2].gesture == brk &&
                    d.lanes[2].amount == c.amount && d.lanes[2].selection == 0x01 && d.lanes[2].target == kVolume && d.lanes[2].count == c.count &&
                    d.lanes[0].selection == 0x14 && d.lanes[1].selection == 0x20,
                "a variation with three lanes: all three back on reopening");
        again.close(store);
    }
    std::puts("variations three lanes: ok");
    return 0;
}
