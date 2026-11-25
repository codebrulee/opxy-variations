// Host test: the variation page (page.hpp): opening on a join, each encoder
// (move, gesture, target, amount; Shift: snap, length, encoders 3 and 4
// blank; build 022: encoder 2's tap opens the shape page, encoder 1's switches the
// lane, the title "variation 1 of 3" and the join's scenes; build 021: Shift + M4 deletes,
// Shift + encoder presses do nothing; no gesture-level curve, each gesture its own), track keys, -/+ in song order with the header's number, M1's lanes
// (an empty lane b taking lane a's gesture), "no tracks", deleting, the
// store kept live, and every frame valid and within the page's budget
// (extension.json page.display).
#include "../page.hpp"
#include "sketch.hpp"
#include "texts.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
using namespace variations;
using opxy::extensions::PageInput;
namespace draw = opxy::draw;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
// examples/first-test.json (vartest's variations at the join into entry 2) as
// build 003 saved it (version 1): three variations once rewritten as version 4.
static const u8 vartest[] = {86, 82, 1, 2, 0, 1, 224, 255, 32, 0, 0, 0, 3, 0, 0, 2, 16, 2, 0, 128, 0, 31, 0, 0, 33, 2, 0, 0, 0, 31, 255, 40, 0, 1, 32, 0, 32, 0, 0, 0, 12, 0, 0, 3, 50, 2, 0, 128, 0, 31, 0, 196, 3, 2, 0, 255, 0, 31, 0, 0, 19, 3, 0, 128, 0, 15, 255, 0, 31, 255, 0};
static Store store;
static Heard heard;
static VariationPage page;
static PageFrame frame;
static const i32 unit = draw::unit;

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
static PageInput click(u32 encoder, bool shift = false) {
    PageInput in = {};
    in.kind = PageInput::click, in.encoder = encoder, in.key = keys::encoder + encoder, in.shift = shift;
    return in;
}
// The SDK's names (Actions::track_target_label), faked: every track a stock
// synth naming slots 1, 2 and 4 "shape", "ratio", "" (no name: its position)
// and none for 3 and 5..8 (no control), except track 6, an SDK synth whose
// slot 1 is a choice; fixed labels as the SDK's table.
static bool fake_name(u32 track, u32 word, char* text, u32 capacity) {
    const opxy::extensions::TrackTarget* t = opxy::extensions::track_target(word);
    if (!t || track >= kTracks || capacity < 8) return false;
    const char* name = t->label;
    if (!name) {
        static const char* const slots[8] = {"shape", "ratio", nullptr, "", nullptr, nullptr, nullptr, nullptr};
        name = track == 5 && word == 0 ? nullptr : slots[word];
        if (!name) return text[0] = 0, false;
        if (!*name) name = word == 3 ? "m1 4" : "m1 ?";
    }
    std::strncpy(text, name, capacity - 1);
    text[capacity - 1] = 0;
    return true;
}
static const Names names = {fake_name};
static void give(const PageInput& in) { require(!page.input(store, in, &names), "the page never closes itself"); }
static void give_to(Store& s, const PageInput& in) { require(!page.input(s, in, &names), "the page never closes itself"); }
static Around song_1(u32 join_entry) {
    Around a = {};
    a.song = true, a.song_index = 0, a.entries = 2, a.entry = join_entry;
    a.scene_before = join_entry ? 0 : 1, a.scene_after = join_entry ? 1 : 0;
    a.steps_before = 64, a.steps_after = 64;
    return a;
}
static const char* text(u32 slot) { return frame.texts[slot].chars; }
static bool visible(const draw::Node& n) { return n.x != draw::hidden_x * unit; }
static u32 shown(u32 from, u32 count) {
    u32 n = 0;
    for (u32 i = from; i < from + count; ++i) n += visible(frame.nodes[i]);
    return n;
}

// Every frame: valid, everything shown on screen, texts within it (estimated),
// the outline a closed ribbon within its node's reach, the window's tiles exactly
// spanning it.
static void frame_ok(bool reset = true) {
    PageTick tick = {16, reset};
    page.draw(tick, frame);
    require(draw::valid(frame), "a valid frame");
    for (u32 i = 0; i < screen::nodes; ++i) {
        const draw::Node& n = frame.nodes[i];
        if (!visible(n)) continue;
        require(n.x >= 0 && n.x <= 480 * unit && n.y >= 0 && n.y <= 220 * unit, "shown nodes on screen");
    }
    // Texts (build 022): stock's sizes, within their rooms, apart (tests/texts.hpp).
    texts_check::texts_ok(frame, page.on_points());
    for (u32 o = 0; o < screen::paths; ++o) {
    const auto& path = frame.paths[o];
    require(path.key != 0, "the outline has a key");
    for (u32 i = 0; i < screen::points; ++i) {
        const draw::Point& p = path.points[i];
        require(p.x >= -64 * unit && p.x <= 64 * unit && p.y >= -64 * unit && p.y <= 64 * unit, "outline within reach");
        const draw::Point& q = path.points[(i + 1) % screen::points];
        // (Build 049: in 64 bits: a plot-high jump between samples, 46 px a side of the node, squares past 32 bits.)
        const long long dx = p.x - q.x, dy = p.y - q.y;
        require(dx * dx + dy * dy >= static_cast<long long>(unit) * unit / 64, "outline points at least 1/8 px apart");
    }
    for (u32 i = 0; i < screen::points / 2; ++i) {
        const draw::Point& top = path.points[i];
        const draw::Point& bottom = path.points[screen::points - 1 - i];
        const i32 dx = top.x - bottom.x, dy = top.y - bottom.y;
        require(dx * dx + dy * dy >= unit * unit / 4 && dx * dx + dy * dy <= unit * unit, "a ribbon 0.5..1 path units thick");
    }
    }
    if (page.on_points()) return;  // the shape page's line is nodes 0 and 1 (tests/points.cpp)
    // (build 047: a muted lane's curve is the dim grey either way)
    const DraftLane& shown_lane = page.draft().lanes[page.lane_index()], &other_lane = page.draft().lanes[page.lane_index() ^ 1];
    // (Build 050: a curve wider than a path reaches is the lane shown across its two paths: n_shape, n_shape_b.)
    // Build 060: the other lanes are sketched from nodes, never from paths: n_other and n_other_b are always hidden, and
    // each other lane's pieces are all drawn or none, in its dim shade, only when it has something to draw.
    const u32 shown_shade = shown_lane.muted ? screen::s_dim : screen::s_bright;
    (void)other_lane;
    for (u32 node : {screen::n_other, screen::n_other_b})
        require(!visible(frame.nodes[node]), "build 060: no other lane on the paths");
    for (u32 lane = 0; lane < kLanes; ++lane) {
        if (lane == page.lane_index()) continue;
        const DraftLane& o = page.draft().lanes[lane];
        const u32 n = sketch_check::pieces_shown(frame, page.lane_index(), lane);
        require(n == 0 || n == screen::sketch_pieces, "a sketch is all its pieces or none");
        require(n == 0 || lane_moves(o), "a lane with nothing to draw has no sketch");
        require(sketch_check::pieces_shade(frame, page.lane_index(), lane, o.muted ? screen::s_dim : screen::s_other), "the other lanes dim");
    }
    for (u32 node : {screen::n_shape, screen::n_shape_b})
        require(!visible(frame.nodes[node]) || frame.nodes[node].shade == shown_shade, "the lane bright");
    require(!visible(frame.nodes[screen::n_shape_b]) || visible(frame.nodes[screen::n_shape]), "a second half only with a first");
}
// Build 049: the timeline across the plot (x 24..456): step `step` of the view [from, to) steps from the join, Q10.
// (The page's own is in eighths of a pixel: compare with near.)
static i32 tl_x(i32 step, i32 from, i32 to) { return 24 * unit + (step - from) * 432 * unit / (to - from); }
static bool near_x(i32 a, i32 b) { return a - b <= unit / 4 && b - a <= unit / 4; }
// The lane shown's curve: its ribbon's first and last top points are the window's ends in the view.
static void curve_spans(i32 a, i32 z) {
    const draw::Node& n = frame.nodes[screen::n_shape];
    const draw::Path<screen::points>& path = frame.paths[screen::n_shape];
    require(visible(n) && n.scale == 2 * unit, "the lane's curve shown");
    const i32 first = n.x + path.points[0].x * 2, last = n.x + path.points[screen::points / 2 - 1].x * 2;
    if (!near_x(first, a) || !near_x(last, z)) {
        std::fprintf(stderr, "curve from %d to %d, wants %d to %d (1/1024 px)\n", first, last, a, z);
        require(false, "the curve spans the window's place in the view");
    }
}
// The lane letters (build 021; build 060: a, b and c): the shown one's ring and stem white (c's glyph white), the others dim.
static bool letters_show(u32 lane) {
    using screen::n_ring_a; using screen::n_stem_a; using screen::n_ring_b; using screen::n_stem_b; using screen::n_letter_c;
    using screen::s_ring_white; using screen::s_white; using screen::s_ring_mid; using screen::s_mid;
    if (!visible(frame.nodes[n_ring_a]) || !visible(frame.nodes[n_stem_a]) || !visible(frame.nodes[n_ring_b]) ||
        !visible(frame.nodes[n_stem_b]) || !visible(frame.nodes[n_letter_c]))
        return false;
    const bool a = lane == 0, b = lane == 1, c = lane == 2;
    return frame.nodes[n_ring_a].shade == (a ? s_ring_white : s_ring_mid) && frame.nodes[n_stem_a].shade == (a ? s_white : s_mid) &&
           frame.nodes[n_ring_b].shade == (b ? s_ring_white : s_ring_mid) && frame.nodes[n_stem_b].shade == (b ? s_white : s_mid) &&
           frame.nodes[n_letter_c].shade == (c ? s_white : s_mid) && frame.nodes[n_ring_b].x > frame.nodes[n_ring_a].x &&
           frame.nodes[n_letter_c].x > frame.nodes[n_ring_b].x;
}
static u32 shown_nodes() {
    u32 n = 0;
    for (u32 i = 0; i < screen::nodes; ++i) n += visible(frame.nodes[i]);
    return n;
}

int main() {
    // Build 052: the strip's words are fitted to 55 px at 15 px, by the device's measure; off the device the estimate is
    // wider than it (texts_check::device_like). The estimate's own checks below take the measure off.
    draw::host_text_measure() = texts_check::device_like;
    // vartest: song 1 = scene 1, scene 2; three custom variations at the join into entry 2
    // (its second had three lines: the third is a variation of its own now).
    require(store.load(vartest, sizeof vartest) && store.count() == 3, "vartest loads, as version 6");
    page.open(store, song_1(1), heard, &names);
    require(page.showing() && page.stored() && page.index() == 0 && page.lane_index() == 0, "opens on the join's first, lane a");
    const Draft& d = page.draft();
    const DraftLane& a = d.lanes[0];
    const DraftLane& b = d.lanes[1];
    // (Build 024, format version 5: a custom lane's tracks are those that had points, track 1 here.)
    require(a.gesture == custom && d.offset == -32 && d.length == 32 && a.selection == 0x01 && a.target == kTargetNotes, "as saved");
    give(press(keys::track + 1));
    require(a.gesture == custom && a.selection == 0x03 && store[0].lanes[0].selection == 0x03, "track 2 added: one line, both follow it");
    frame_ok();
    require(!std::strcmp(text(screen::t_title), "variation 1 of 3") && !std::strcmp(text(screen::t_join), "scene 1 > scene 2"), "header: 1 of the song's 3, the sections");
    // Build 056: no track list in the header; the LEDs (below) carry the tracks, and "mute" is over M2.
    require(letters_show(0) && !std::strcmp(text(screen::t_mark_m2), "mute"), "lane a shown; \"mute\" over M2 (build 056)");
    require(!std::strcmp(text(screen::t_value_2), "edit") && !std::strcmp(text(screen::t_value_3), "notes"), "gesture and target (build 053: \"edit\")");
    require(!std::strcmp(text(screen::t_value_1), "-2b") && !std::strcmp(text(screen::t_value_3), "notes") &&
                page.target_place() == 1 && page.target_count() == 25,
            "values: move, target by name alone (build 024; the notes first of 25: build 032, a two-track lane lists no engine names)");
    require(!std::strcmp(text(screen::t_name_1), "move"), "build 053: the name is \"move\"; the snap is the Shift layer's");
    // Build 049: the timeline. A 32 step window in a 64 step view (twice its length), centred on the join (its middle
    // is 16 steps before it): steps -32..32; the window, the left half of the plot, the join at its middle.
    curve_spans(tl_x(-32, -32, 32), tl_x(0, -32, 32));
    require(near_x(frame.nodes[screen::n_join].x, tl_x(0, -32, 32)) && frame.nodes[screen::n_join].shade == screen::s_text &&
                frame.nodes[screen::n_join].y == screen::tl_cy * unit, "the join at step 0, in the middle of the plot, in s_text (build 052)");
    require(visible(frame.nodes[screen::n_shape]) && sketch_check::pieces_shown(frame, 0, 1) == screen::sketch_pieces &&
                sketch_check::pieces_shown(frame, 0, 2) == 0,
            "both lanes drawn: the shown one on its path, lane b sketched, lane c (empty) nothing");
    require(shown_nodes() <= 4 + 3 + 16 + 28 + 1 + 1 + 3 + 1 + 4 + 1 + 1 + screen::sketch_lanes * screen::sketch_pieces + screen::cell_count &&
                !visible(frame.nodes[screen::n_playhead]) &&
                !visible(frame.nodes[screen::n_cursor]),
            "within the declared nodes; no playhead while stopped");
    require(shown(screen::n_marks, screen::mark_count) == 0 && shown(screen::n_bars, screen::bar_count) == 0 && !visible(frame.nodes[screen::n_floor]) &&
                shown(screen::n_handles, screen::handle_count) == 0,
            "no other joins in view; no ruler marks, tiles, floor or handles");
    require(!visible(frame.nodes[screen::n_start]), "build 054: no ruler line in the artwork, so no strip over it");
    require(shown(screen::n_as_set, screen::as_set_count) == screen::as_set_count, "the as set line dashed across the plot");
    // Build 052 (2026-11-24): the control strip's eight cells, the M2 and M3 chevrons, the M4 mark.
    {
        using namespace screen;
        using screen::unit;
        // The eight cells, 60 x 20 black, at (60 i + 30, 10), the ramp's greys left to right (#000000 #323232 #555555 #646464
        // #7D7D7D #969696 #B4B4B4 #CDCDCD: s_black s_dim s_grid s_mid s_ramp s_text s_join s_bright).
        require(n_cells == 163 && cell_count == 8 && n_letter_c == 171 && n_sketch == 172 && sketch_lanes == 2 && sketch_pieces == 31 &&
                    nodes == 234 && nodes <= 256 && texts == 16 && palette == 16,
                "build 054's node ids and counts (052's four chevrons are gone); build 060: c at 171, the sketches from 172, 234 nodes of 256");
        const u32 greys[8] = {s_black, s_dim, s_grid, s_mid, s_ramp, s_text, s_join, s_bright};
        for (u32 i = 0; i < cell_count; ++i) {
            const draw::Node& c = frame.nodes[n_cells + i];
            require(visible(c) && c.x == (60 * static_cast<i32>(i) + 30) * unit && c.y == 10 * unit && c.scale == unit && c.shade == greys[i],
                    "a strip cell centred at (60 i + 30, 10) in its ramp grey");
        }
        require(s_ramp == palette - 1, "the ramp's grey is the last palette entry");
        // The strip's names and values: left aligned at x 5, 125, 245, 365 and 65, 185, 305, 425; baselines 10 and 16.5.
        for (u32 c = 0; c < 4; ++c) {
            require(frame.texts[t_name_1 + c].x == (5 + 120 * static_cast<i32>(c)) * unit && frame.texts[t_name_1 + c].y == 10 * unit &&
                        frame.texts[t_value_1 + c].x == (65 + 120 * static_cast<i32>(c)) * unit &&
                        frame.texts[t_value_1 + c].y == 33 * unit / 2 && frame.texts[t_name_1 + c].chars[0] && frame.texts[t_value_1 + c].chars[0],
                    "the strip's anchors, 16.5 px as 33 * unit / 2");
            // Build 053 ("move beat" "doesn't really fit, which is why I'm suggesting this"): a name leaves its
            // cell air too, not just a value. "move beat" was 46 px of the 55 and "move step" 45; "move" is 24, and
            // every other name is 36 or under ("remove" the widest, 33.6 px, since build 064).
            require(words::width(frame.texts[t_name_1 + c].chars, names_size) <= (strip_room - 9) * unit &&
                        words::width(frame.texts[t_value_1 + c].chars, value_size) <= (strip_room - 9) * unit,
                    "every strip name and value leaves 9 px of its cell at its size");
        }
        // Build 054: "back" over M4 (stock closes the page on it) and "shape" over M3, both at stock's anchors on
        // baseline 215; the lane letters at M1, x 28.8, 3/2 scale.
        // Build 055: the marks are centred on their keys (within a pixel), "edit" over M3 now.
        {
            auto centred = [&](u32 slot, i32 centre) {
                const draw::Text& t = frame.texts[slot];
                const i32 d = t.x + words::width(t.chars, mark_size) / 2 - centre;
                return (d < 0 ? -d : d) <= unit && t.y == m_y * unit && t.x >= 0 && t.x + words::width(t.chars, mark_size) <= 480 * unit;
            };
            require(!std::strcmp(frame.texts[t_mark].chars, "back") && centred(t_mark, m4_x), "\"back\" centred over M4 without Shift");
            require(!std::strcmp(frame.texts[t_mark_side].chars, "edit") && centred(t_mark_side, m3_x),
                    "build 055: \"edit\" centred over M3 without Shift");
            // Build 056: "mute" over M2 on this page too, the third mark, centred like the others.
            require(!std::strcmp(frame.texts[t_mark_m2].chars, "mute") && centred(t_mark_m2, m2_x), "build 056: \"mute\" centred over M2");
        }
        require(frame.texts[t_title].y == 38 * unit && frame.texts[t_join].y == 38 * unit, "the header's baseline 38");
        // Build 053: the chart 6 px up (tl_dy 4, was 10). Build 054: no ruler, the shape page's bar labels at 186.
        require(plot_y0 == 72 && plot_y1 == 164 && plot_cy == 118 && tl_dy == 4 && tl_y0 == 76 && tl_y1 == 168 && tl_cy == 122 &&
                    bar_label_y == 186 && points_target_y == 50 && points_message_y == 50,
                "build 054's geometry");
        require(frame.texts[t_label_a].y == 68 * unit && frame.texts[t_number_a].y == 181 * unit, "the scene names' and bar numbers' baselines 68 and 181");
    }
    // Build 021's quiet hints: the handles, and the encoder dots. Build 054 ("no need for the duplicate actions
    // on the pressed encoders now", "why is there a small hollow circle next to a/b"): both dots are gone with the
    // taps they hinted at - the lane is M1's and the shape page M3's, each with a word or the letters over its key.
    {
        using screen::n_handles; using screen::handle_count; using screen::n_enc2; using screen::n_enc1; using screen::n_ring_a;
        using screen::letters_y; using screen::letters_x; using screen::join_x;
        require(shown(n_handles, handle_count) == 0, "build 049: no handles on the timeline");
        require(!visible(frame.nodes[n_enc2]), "build 054: no encoder 2 dot (the shape page is M3's)");
        // Build 052: the letters at M1's anchor (x 28.8, baseline 215), drawn at 3/2 scale; the join's scenes stay top left.
        const i32 s = screen::letters_scale;
        require(letters_show(0) && frame.nodes[n_ring_a].x > letters_x && frame.nodes[n_ring_a].x < letters_x + 10 * s &&
                    frame.nodes[n_ring_a].y < letters_y * unit && frame.nodes[n_ring_a].y > (letters_y - 15) * unit &&
                    frame.nodes[n_ring_a].scale == s && frame.nodes[screen::n_stem_b].scale == s && frame.nodes[screen::n_letter_c].scale == s &&
                    frame.texts[screen::t_join].x == join_x * unit && frame.texts[screen::t_join].y == screen::header_y * unit,
                "a bright, b and c dim, at the M1 mark at 3/2 scale; the scenes top left");
        {
            // Build 055: the letters centred on M1's key, measured off the drawn nodes themselves.
            // Build 064 ("c is still just a little too far over to the right"): by the REAL ink, in px at scale 1
            // from each glyph's centre: bowls +/-3.1, c -3.0882 / +1.9926 (not the nominal 2.6 either side, which sat
            // the row 0.55 px left of the key).
            const i32 bowl = 31 * s / 10;
            const i32 edge_a = frame.nodes[n_ring_a].x - bowl;
            const i32 edge_c = frame.nodes[screen::n_letter_c].x + 19926 * s / 10000;
            const i32 d = (edge_a + edge_c) / 2 - screen::m1_x;
            require(d >= -unit / 4 && d <= unit / 4, "build 064: the lane letters' ink centred on M1's key, within a quarter pixel");
        }
        require(!visible(frame.nodes[n_enc1]) &&
                    frame.nodes[screen::n_ring_b].x == letters_x + screen::letter_pitch * s + (screen::ring_r10 + 5) * s / 10,
                "build 054: no encoder 1 dot; b still a letter pitch right of a");
        {
            // Build 062 ("the c has a bigger gap next to b than a does to b though, can it be made equal").
            // Build 064 ("c is still just a little too far over to the right"): measured by the real ink (see
            // page.hpp): the bowls reach 3.1 px from their centres, stems 0.3, c -3.0882 / +1.9926. a-b is 5.8 px;
            // b-c is set tighter, by 1.8 px (4.0 px), because a bowl facing an arc reads looser than two stems
            // facing across a constant gap. So the test asserts that relationship, not equality.
            const i32 bowl = 31 * s / 10, half_stem = 3 * s / 10;
            const i32 a_right = std::max(frame.nodes[screen::n_stem_a].x + half_stem, frame.nodes[n_ring_a].x + bowl);
            const i32 b_left = frame.nodes[screen::n_stem_b].x - half_stem;
            const i32 b_right = frame.nodes[screen::n_ring_b].x + bowl;
            const i32 c_left = frame.nodes[screen::n_letter_c].x - 30882 * s / 10000;
            const i32 gap_ab = b_left - a_right, gap_bc = c_left - b_right;
            const i32 tighter = gap_ab - gap_bc, want = 18 * s / 10;
            if (tighter < want - unit / 8 || tighter > want + unit / 8 || gap_ab < 57 * s / 10 || gap_ab > 59 * s / 10) {
                std::fprintf(stderr, "a-b gap %d/1024 px, b-c gap %d/1024 px\n", gap_ab, gap_bc);
                require(false, "build 064: a-b 5.8 px and b-c 1.8 px tighter than it");
            }
            require(gap_ab > 0 && gap_bc > 0, "build 062: and the letters do not touch");
        }
    }
    require(!page.fresh(), "a stored one is not NEW");
    PageLeds leds = {};
    page.leds(leds);
    require(leds.track[0] == led::on && leds.track[1] == led::on && leds.track[2] == led::off, "selection on the keys");

    // A plain tap of encoder 1 (build 022; 020-021's encoder 2; M1 too): lane b, its tracks, target
    // and LEDs; the other lane drawn dim behind; b bright now, a dim (build 021).
    // Build 056: plain M2 mutes the lane shown (it was Shift + M1). Build 057 ("I didn't ask for this
    // change?"): Shift + M1 switches lane, as plain M1 does, and mutes nothing.
    give(press(keys::m1, true));
    require(page.lane_index() == 1 && !page.on_points() && !a.muted && !b.muted, "build 057: Shift + M1 switches lane, mutes nothing");
    give(press(keys::m1, true));
    require(page.lane_index() == 2 && !page.on_points() && !a.muted && !b.muted && !page.draft().lanes[2].muted, "build 060: Shift + M1 on to lane c, mutes nothing");
    give(press(keys::m1, true));
    require(page.lane_index() == 0 && !a.muted, "and round to a again");
    give(press(keys::m1 + 1));
    require(page.lane_index() == 0 && !page.on_points() && a.muted, "M2 mutes the lane");
    give(press(keys::m1 + 1));
    require(!a.muted, "and again unmutes it");
    give(press(keys::m1));
    require(page.lane_index() == 1 && b.target == kSend && b.selection == 0x02, "lane b: fx i send on track 2");
    frame_ok();
    require(letters_show(1) && b.selection == 0x02 &&
                !std::strcmp(text(screen::t_value_3), "fx i") && !std::strcmp(text(screen::t_value_2), "edit"),
            "lane b shown (build 052: 'fx i send' shortened to 'fx i' in the strip)");
    page.leds(leds);
    require(leds.track[0] == led::off && leds.track[1] == led::on, "lane b's tracks on the keys");
    require(!page.fresh() && store[0].lanes[1].target == kSend, "switching lanes is no edit");
    give(press(keys::m1));
    require(page.lane_index() == 2, "the tap again: lane c (build 060)");
    give(press(keys::m1));
    require(page.lane_index() == 0, "M1 too: round to lane a");
    give(press(keys::m1));
    require(page.lane_index() == 1, "lane b");
    give(press(keys::m1)), give(press(keys::m1));
    require(page.lane_index() == 0, "and back to a (c, then a)");

    // Encoder 2: a gesture replaces lane a's custom points; kept in the store at once; lane b kept.
    give(turn(1, 1));
    require(a.gesture == brk && store[0].lanes[0].gesture == brk && store.count() == 3, "break, stored live");
    require(a.target == kTargetNotes && !a.default_target, "its own target kept (chosen, not the gesture's)");
    u32 n;
    require(store.points(store[0], 0, &n) && n == 3, "break's line in the store");
    require(!a.default_tracks && a.selection == 0x03, "its own tracks kept");
    require(b.gesture == custom && store.points(store[0], 1, &n) && n == 2, "lane b untouched");
    frame_ok();
    require(!std::strcmp(text(screen::t_value_2), "break"), "break shown");
    // Through every gesture and back; the ends hold.
    const u32 order[] = {brk, build, drop, cut_in, gate, stutter, stutter_up, stutter_spring, stutter_down, fade_in, fade_out, hold, rise, fall, swell, dip, random_, wander};
    for (u32 i = 1; i < 20; ++i) {  // build 062: hold joins the list, one more detent
        give(turn(1, 1)), frame_ok();
        require(a.gesture == order[i < 18 ? i : 17] && a.gesture != kNoGesture, "lane a: the gestures in order, never none");
        if (a.gesture == hold)  // build 062: hold sits after the fades, reads "hold", keeps the curve at no bend
            require(!std::strcmp(text(screen::t_value_2), "hold") && a.curve == 0 && order[i - 1] == fade_out, "hold on the strip, after fade out");
        // Build 039: the fades keep a chosen target too (016-038 put them on the volume).
        require(a.target == kTargetNotes && !a.default_target, "every gesture, the fades too: the chosen target kept");
    }
    require(a.gesture == wander && store[0].lanes[0].gesture == wander && !std::strcmp(text(screen::t_value_2), "wand"), "wander at the end (build 053: \"wand\")");
    for (u32 i = 0; i < 20; ++i) give(turn(1, -1)), frame_ok(false);
    require(a.gesture == brk && a.target == kTargetNotes, "break at the start, its target kept");
    give(turn(1, 9));
    require(a.gesture == fade_in && store[0].lanes[0].gesture == fade_in, "one gesture a detent: fade in");
    give(turn(1, -9));

    // Encoder 4: lane a's amount, 1 % a detent, 5 fast.
    const u32 exit_before = store.points(store[0], 0, &n)[n - 1].step;
    give(turn(3, 10, false, 2));
    require(!std::strcmp((frame_ok(false), text(screen::t_value_4)), "50%"), "amount 0 to 50 %, fast");
    require(store[0].lanes[0].amount == a.amount && store.points(store[0], 0, &n)[n - 1].step != exit_before, "spreads the exits");
    give(turn(3, -1));
    frame_ok(false);
    require(!std::strcmp(text(screen::t_value_4), "49%"), "amount 49 %");
    for (u32 i = 0; i < 30; ++i) give(turn(3, 10, false, 2));
    require(a.amount == 255 && b.amount == 0, "amount up to 100 %, lane a's only");

    // Shift + encoder 2: length (build 020; was encoder 3), keeping the window's end on the join.
    give(turn(1, -1, true));
    require(d.length == 16 && d.offset == -16 && store[0].length == 16, "1 bar, ending on the join");
    require(store.points(store[0], 1, &n) && store.points(store[0], 1, &n)[n - 1].step == 15, "lane b's custom points rescaled");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_2), "bar") && !std::strcmp(text(screen::t_value_2), "1"), "1 bar: name \"bar\", value 1 (build 052)");
    give(turn(1, -1, true));
    require(d.length == 8 && d.offset == -8, "half a bar");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_2), "beats") && !std::strcmp(text(screen::t_value_2), "2"), "2 beats: \"beats\" and 2");
    give(turn(1, -1, true));
    require(d.length == 8, "no shorter");
    for (u32 i = 0; i < 4; ++i) give(turn(1, 1, true));
    require(d.length == 128 && d.offset == -128, "8 bars");
    frame_ok();
    // Build 047: on to 16 bars (256 steps).
    give(turn(1, 1, true));
    require(d.length == 256 && d.offset == -256 && store[0].length == 256, "16 bars, ending on the join");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_2), "bars") && !std::strcmp(text(screen::t_value_2), "16"), "16 bars: \"bars\" and 16");
    // Build 049: on to 32 bars (512 steps) and 64 (1024); build 063: and on, below.
    give(turn(1, 1, true));
    require(d.length == 512 && d.offset == -512 && store[0].length == 512, "32 bars, ending on the join");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_2), "bars") && !std::strcmp(text(screen::t_value_2), "32"), "32 bars: \"bars\" and 32");
    give(turn(1, 1, true));
    require(d.length == 1024 && d.offset == -1024 && store[0].length == 1024, "64 bars, ending on the join");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_2), "bars") && !std::strcmp(text(screen::t_value_2), "64"), "64 bars: \"bars\" and 64");
    require(store.points(store[0], 1, &n)[n - 1].step < 256, "lane b's custom points stay in the 256-step line");
    // Build 063 (version 12: "I'd like a song spanning variation to be an option"): on to 128 bars (2048) and 256 (4096), no longer
    // (the old test stopped at 1024 here).
    give(turn(1, 1, true));
    require(d.length == 2048 && d.offset == -2048 && store[0].length == 2048, "128 bars, ending on the join");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_2), "bars") && !std::strcmp(text(screen::t_value_2), "128"), "128 bars: \"bars\" and 128");
    give(turn(1, 1, true));
    require(d.length == 4096 && d.offset == -4096 && store[0].length == 4096, "256 bars, ending on the join");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_2), "bars") && !std::strcmp(text(screen::t_value_2), "256"), "256 bars: \"bars\" and 256");
    give(turn(1, 1, true));
    require(d.length == 4096, "no longer");
    give(turn(1, -7, true));
    require(d.length == 32 && d.offset == -32, "back to 2 bars");
    // Build 041: Shift + encoder 3's turn is the lane's rate: 1x, 2x, 4x, 8x, clamped, each step stored.
    require(a.rate == 1 && store[0].lanes[0].rate == 1, "rate 1x to start with");
    give(turn(2, -1, true));
    require(a.rate == 1 && store[0].lanes[0].rate == 1, "1x: no lower");
    give(turn(2, 1, true));
    require(a.rate == 2 && store[0].lanes[0].rate == 2 && store[0].lanes[1].rate == 1, "Shift + encoder 3: 2x, lane a's, stored");
    give(turn(2, 1, true));
    require(a.rate == 4 && store[0].lanes[0].rate == 4, "4x");
    give(turn(2, 1, true));
    require(a.rate == 8 && store[0].lanes[0].rate == 8, "8x");
    give(turn(2, 1, true));
    require(a.rate == 8 && store[0].lanes[0].rate == 8, "8x: no higher");
    frame_ok(false);
    require(!std::strcmp(text(screen::t_name_3), "rate") && !std::strcmp(text(screen::t_value_3), "8x"), "the Shift row shows rate 8x");
    give(turn(2, -1, true)), give(turn(2, -1, true));
    require(a.rate == 2 && store[0].lanes[0].rate == 2, "back down to 2x");
    give(turn(2, -1, true));
    require(a.rate == 1 && store[0].lanes[0].rate == 1 && d.length == 32 && d.offset == -32 && store[0].length == 32,
            "and 1x; the window untouched");

    // Encoder 1: slide by the snap, within the sections; Shift + encoder 1: the snap.
    give(turn(0, 1));
    require(d.offset == -16 && store[0].offset == -16, "a bar later: straddles the join");
    give(turn(0, 1));
    require(d.offset == 0, "on the join");
    frame_ok();
    require(!std::strcmp(text(screen::t_title), "variation 1 of 3") && !std::strcmp(text(screen::t_join), "scene 2") && !std::strcmp(text(screen::t_value_1), "join"),
            "inside scene 2");
    curve_spans(tl_x(0, -32, 32), tl_x(32, -32, 32));  // the window slid right inside the view: the view stays
    require(near_x(frame.nodes[screen::n_join].x, tl_x(0, -32, 32)), "the view still while the window moves inside it");
    for (u32 i = 0; i < 10; ++i) give(turn(0, 1));
    require(d.offset == 32, "no later than the section's end (64 - 32)");
    for (u32 i = 0; i < 10; ++i) give(turn(0, -1));
    // Build 024: as far left as right, wholly in the section before (to its start), still on this join.
    require(d.offset == -64 && store[0].offset == -64 && store[0].entry == 1, "no earlier than the section before's start (-64)");
    frame_ok();
    require(!std::strcmp(text(screen::t_value_1), "-4b"), "-4 bars (build 053: the unit one letter, \"-4b\")");
    // The window left the view at its left: the view shifts just enough that the window touches that edge (and no
    // further than the song: 64 steps before the join).
    curve_spans(tl_x(-64, -64, 0), tl_x(-32, -64, 0));
    require(near_x(frame.nodes[screen::n_join].x, tl_x(0, -64, 0)), "the view followed the window");
    for (u32 i = 0; i < 2; ++i) give(turn(0, 1));
    require(d.offset == -32, "back to ending on the join");
    give(turn(0, -1, true));
    frame_ok();
    require(page.snap() == 4 && !std::strcmp(text(screen::t_name_1), "snap") && !std::strcmp(text(screen::t_value_1), "beat"),
            "snap: beat (Shift held: the Shift layer)");
    give(turn(0, 1));
    require(d.offset == -28, "a beat later");
    frame_ok();
    require(!std::strcmp(text(screen::t_name_1), "move"), "build 053: \"move\" whatever the snap, without Shift");
    require(!std::strcmp(text(screen::t_value_1), "-28s"), "-28 steps (build 053: \"-28s\")");
    give(turn(0, -1, true));
    give(turn(0, 1));
    require(page.snap() == 1 && d.offset == -27, "snap: step");
    give(turn(0, 1, true));
    give(turn(0, 1));
    require(page.snap() == 4 && d.offset == -24, "to the beat's grid first");
    give(turn(0, 1, true));
    give(turn(0, -1));
    require(page.snap() == 16 && d.offset == -32, "and the bar's");
    give(turn(0, 1, true));
    require(page.snap() == 16, "bar is the last snap");

    // Track keys: in and out of lane a, on the LEDs; lane b's tracks untouched.
    give(press(keys::track + 4));
    require(a.selection == 0x13 && store[0].lanes[0].selection == 0x13 && store[0].lanes[0].count == 3, "track 5 joins");
    require(b.selection == 0x02, "lane b's tracks are its own");
    page.leds(leds);
    require(leds.track[4] == led::on, "its key lit");
    give(press(keys::track + 0));
    require(a.selection == 0x12 && !(store[0].lanes[0].selection & 1), "track 1 leaves");
    frame_ok();
    page.leds(leds);
    require(leds.track[0] == led::off && leds.track[1] == led::on && leds.track[4] == led::on, "tracks 2 5: their keys lit (build 056: no header list)");
    give(press(keys::track + 1));
    give(press(keys::track + 4));
    frame_ok();
    // Build 056: the header no longer says "no tracks"; the strip's target column does: "tracks" / "none".
    require(a.selection == 0 && !std::strcmp(text(screen::t_name_3), "tracks") && !std::strcmp(text(screen::t_value_3), "none"),
            "none: the target column says tracks / none");
    require(!visible(frame.nodes[screen::n_shape]), "no motion drawn for a lane with no tracks");
    give(press(keys::track + 2));

    // Build 020's Shift + encoder 3 and 4 presses: nothing now (build 021).
    // Encoder 2's tap (build 022): the shape page (tests/points.cpp), and again there: back.
    {
        const u32 count = store.count();
        give(click(2, true)), give(click(1, true));
        require(!page.on_points() && store.count() == count && page.stored() && !page.message(),
                "Shift + encoder 2 or 3 pressed: nothing");
        give(click(3, true));
        require(!page.on_points() && store.count() == count && !a.muted, "Shift + encoder 4's tap: nothing (build 048)");
    }
    give(press(keys::m1 + 2));
    require(page.on_points(), "shape");
    frame_ok();
    give(click(2, true)), give(click(3, true)), give(click(1, true)), give(click(3));
    require(page.on_points(), "Shift + encoder 2, 3 or 4 pressed there, or encoder 4's tap: nothing");
    require(!page.draft().lanes[0].muted && !page.draft().lanes[1].muted, "and the shape page's Shift tap mutes nothing");
    give(press(keys::m1 + 2));
    require(!page.on_points(), "and back");
    frame_ok();
    // A lane with no tracks has a shape page from build 057 (it had none before). Build 053: and says nothing about it - the tracks line already
    // reads "no tracks" top right, and 052's message place (the bottom row) made the repeat read as something new.
    give(press(keys::track + 2));
    give(press(keys::m1 + 2));
    require(page.on_points() && !page.message(), "build 057: no tracks: the shape page opens all the same, and nothing is said");
    give(press(keys::m1 + 2));
    require(!page.on_points(), "and back");
    frame_ok(false);
    require(std::strcmp(text(screen::t_number_a), "no tracks"), "the bar number's slot holds a bar, not a message");
    require(!std::strcmp(text(screen::t_name_3), "tracks") && !std::strcmp(text(screen::t_value_3), "none"), "the strip says it (build 056)");
    give(press(keys::track + 2));

    // - and +: the join's other variations, and back.
    give(press(keys::plus));
    require(page.index() == 1 && d.offset == 32 && a.gesture == custom && a.target == kVolume, "the second");
    frame_ok();
    require(!std::strcmp(text(screen::t_title), "variation 2 of 3") && !std::strcmp(text(screen::t_join), "scene 2") && !std::strcmp(text(screen::t_value_3), "vol"), "2 of 3");
    require(!page.message(), "no message: the header says which");
    give(press(keys::plus));
    require(page.index() == 2 && page.number() == 3 && a.target == kCutoff && b.gesture == kNoGesture, "the third: vartest's filter line");
    give(press(keys::plus));
    require(page.index() == 0 && page.number() == 1 && a.gesture == brk, "+ past the song's last: round to its first, as edited");
    give(press(keys::minus));
    require(page.index() == 2, "- back round to the last");
    give(press(keys::plus));
    require(page.index() == 0, "and + to the first");
    // Build 054 ("drop the chevrons"): M3 is the shape page, not the next variation; - and + step the variations
    // alone. Build 056: M2 is mute (twice: back as it was), and moves nothing.
    give(press(keys::m1 + 1));
    require(page.index() == 0 && !page.on_points() && a.muted, "M2: mute, the variation unchanged");
    give(press(keys::m1 + 1));
    require(page.index() == 0 && !page.on_points() && !a.muted, "M2 again: unmuted");
    give(press(keys::m1 + 2));
    require(page.index() == 0 && page.on_points(), "M3: the shape page, the variation unchanged");
    give(press(keys::m1 + 2));
    require(page.index() == 0 && !page.on_points(), "M3 again: back to the variation page");

    // Shift held: the Shift layer along the bottom (snap with its value,
    // target with the lane's, encoders 3 and 4 blank); let go: the encoders' own names again.
    frame_ok(false);
    require(!std::strcmp(text(screen::t_name_1), "move") && !std::strcmp(text(screen::t_name_2), "gesture") &&
                (!std::strcmp(text(screen::t_name_3), "target") || !std::strncmp(text(screen::t_name_3), "track ", 6)) &&
                !std::strcmp(text(screen::t_name_4), "amount") && !std::strcmp(text(screen::t_value_3), "notes"),
            "the encoders' names: move, gesture, target (build 032: a one-track lane's \"track n\"), amount");
    PageInput shift_down = press(keys::shift);
    shift_down.shift = true;
    give(shift_down);
    require(page.shift(), "Shift held");
    frame_ok(false);
    require(!std::strcmp(text(screen::t_name_1), "snap") && !std::strcmp(text(screen::t_value_1), "bar") &&
                !std::strcmp(text(screen::t_name_2), "bars") && !std::strcmp(text(screen::t_value_2), "2") &&
                !std::strcmp(text(screen::t_name_3), "rate") && !std::strcmp(text(screen::t_value_3), "1x") &&
                !text(screen::t_name_4)[0] && !text(screen::t_value_4)[0],
            "the Shift layer: snap, the length (build 052: its unit in the name), rate, column 4 blank");
    // Build 052: "delete" over M4 in the 20 px mark slot, under Shift only (stock's M4 anchor: x 406.2, baseline 215).
    {
        const i32 d = frame.texts[screen::t_mark].x + words::width("delete", screen::mark_size) / 2 - screen::m4_x;
        require(!std::strcmp(text(screen::t_mark), "delete") && (d < 0 ? -d : d) <= unit &&
                    frame.texts[screen::t_mark].y == screen::m_y * unit,
                "Shift: \"delete\" centred over M4");
        require(!text(screen::t_mark_side)[0] && !text(screen::t_mark_m2)[0], "build 056: Shift: no other mark (mute is plain M2's)");
    }
    const u32 shift_names[] = {screen::t_name_1, screen::t_name_2, screen::t_name_3, screen::t_name_4};
    for (u32 slot : shift_names)
        require(words::width(text(slot), screen::names_size) <= screen::strip_room * screen::unit, "Shift names fit their cell (build 052: 55 px)");
    require(!std::strcmp(text(screen::t_title), "variation 1 of 3") && !std::strcmp(text(screen::t_join), "scene 1 > scene 2"),
            "the rest as it was");
    give(turn(0, -1, true));
    frame_ok(false);
    require(page.snap() == 4 && !std::strcmp(text(screen::t_value_1), "beat"), "snap's value follows the Shift turn");
    // Encoder 3: the lane's target, through the list (build 025): the notes, param 1..4 (each
    // slot on every lane track, whatever its engine calls it), the engines' controls by name,
    // then the fixed targets. Lane a's tracks are 3 (a stock synth here): its engine names
    // slots 1 and 2 "shape" and "ratio"; slot 4 has no name (its position) and 3, 5-8 no control.
    {
        const Draft was = d;
        char stored[kNameCapacity];
        // Build 032: lane a is track 3 alone, so its list is the notes, its engine's own names
        // (no param slots: those are "all"'s), then the fixed targets.
        give(turn(2, 1));
        frame_ok(false);
        store.name(store[0], 0, stored);
        require(a.target == 0 && !std::strcmp(a.name, "shape") && !a.default_target && !std::strcmp(text(screen::t_value_3), "shape") &&
                    !std::strcmp(stored, "shape") && (store.bytes()[store[0].start + kHeader + 4] & kLaneNamed),
                "notes -> the engine's own shape (slot 1), stored by name (flags bit 3)");
        require(page.target_place() == 2 && page.target_count() == 24, "second of 24: notes, 2 names, unnamed slot 4, 20 fixed");
        require(a.gesture == was.lanes[0].gesture && a.count == was.lanes[0].count, "the gesture and its line stay");
        give(turn(2, 1));
        store.name(store[0], 0, stored);
        require(a.target == 1 && !std::strcmp(page.target_text(), "ratio") && !std::strcmp(stored, "ratio"), "ratio");
        give(turn(2, 1));
        require(a.target == 3 && !a.name[0] && !std::strcmp(page.target_text(), "param 4"),
                "slot 4, a control the engine leaves unnamed: param 4 (build 034)");
        give(turn(2, 1));
        frame_ok(false);
        store.name(store[0], 0, stored);
        require(a.target == 8 && !std::strcmp(page.target_text(), "attack") && !std::strcmp(text(screen::t_value_3), "atk") &&
                    !stored[0] && !(store.bytes()[store[0].start + kHeader + 4] & kLaneNamed),
                "no name for slot 4 (its position) nor 3: the amp envelope next, no name stored");
        for (u32 i = 0; i < 3; ++i) give(turn(2, 1));
        require(a.target == 11 && !std::strcmp(page.target_text(), "release !"), "release, marked costly (build 039)");
        give(turn(2, 1));
        require(a.target == kVolume && !std::strcmp(page.target_text(), "volume"), "volume (M2's Shift row)");
        for (u32 i = 0; i < 15; ++i) give(turn(2, 1)), frame_ok(false);
        require(a.target == 40 && !std::strcmp(text(screen::t_value_3), "pan") && page.target_place() == 24, "pan at the list's end");
        give(turn(2, 1));
        frame_ok(false);
        require(a.target == kTargetNotes && !std::strcmp(text(screen::t_value_3), "notes"), "round to the notes");
        give(turn(2, -1));
        require(a.target == 40, "and back round to pan");
        for (u32 i = 0; i < 2; ++i) give(turn(2, -1));
        require(a.target == 24 && !std::strcmp(page.target_text(), "lfo speed"), "back: lfo speed");
        give(turn(2, -1));
        frame_ok(false);
        require(a.target == 23 && !std::strcmp(page.target_text(), "fx ii send") && !std::strcmp(text(screen::t_value_3), "fx ii"),
                "fx ii send, whole in the model; 'fx ii' in the strip (build 052)");
        require(store[0].lanes[0].target == 23 && store[0].lanes[1].target == kSend, "kept live; lane b's its own");
        for (u32 i = 0; i < 19; ++i) give(turn(2, -1));
        require(a.target == 0 && !std::strcmp(a.name, "shape"), "back to shape");
        give(turn(2, -1));
        require(a.target == kTargetNotes && !a.name[0], "the notes before it: no param slots on one track");
        give(turn(2, 3));  // a fast turn: one choice a detent of direction
        require(a.target == 0 && !std::strcmp(page.target_text(), "shape"), "one choice a turn");
        // Track 6, an SDK synth whose slot 1 is a choice: its names lack shape; back from
        // shape (not listed there) to the last of its names, ratio.
        give(press(keys::track + 2)), give(press(keys::track + 5));
        require(a.selection == 0x20, "lane a on track 6");
        frame_ok(false);
        require(page.target_count() == 23 && page.target_place() == 0, "23 on track 6: no shape (ratio, param 4)");
        give(turn(2, -1));
        require(a.target == 3 && !std::strcmp(page.target_text(), "param 4"), "back from shape: its unnamed slot 4");
        give(turn(2, -1));
        require(a.target == 1 && !std::strcmp(page.target_text(), "ratio"), "ratio: no shape");
        // Build 032: tracks 3 and 6 list "all": no engine's own names (025-031 listed each once);
        // ratio is kept, though not in the list. Track 3 alone lists its own again: shape.
        give(press(keys::track + 2));
        require(a.selection == 0x24 && page.target_count() == 25 && page.target_place() == 0,
                "lane a on tracks 3 and 6: all, 25, no names");
        require(!std::strcmp(page.target_text(), "ratio"), "ratio still");
        give(press(keys::track + 5));
        require(a.selection == 0x04 && page.target_place() > 0, "track 3 alone: ratio listed again");
        give(turn(2, -1));
        frame_ok(false);
        require(a.target == 0 && !std::strcmp(text(screen::t_value_3), "shape") && !std::strcmp(text(screen::t_name_3), "track 3"),
                "shape: track 3's, \"track 3\" over it");
        require(a.selection == 0x04 && !std::strcmp(page.target_text(), "shape"), "back on track 3");
        // No tracks: no params, no names; the target kept but not in the list.
        give(press(keys::track + 2));
        require(!a.selection && page.target_place() == 0 && page.target_count() == 21, "no tracks: no engine controls offered");
        frame_ok(false);
        give(turn(2, 1));
        require(a.target == 8, "on past them: attack");
        give(turn(2, -1));
        require(a.target == kTargetNotes, "back: the notes");
        give(press(keys::track + 2));
        give(turn(2, 1));
        require(a.target == 0 && !std::strcmp(a.name, "shape"), "track 3 again: its shape (build 032; was param 1)");
    }
    {
        // Shift + turning encoder 4 does nothing (encoder 3's is the rate); nor does Shift + an encoder's press.
        const Draft was = d;
        const u32 count = store.count();
        give(turn(3, 5, true, 2)), give(click(1, true));
        give(click(0, true)), give(click(2, true));
        require(d.length == was.length && d.offset == was.offset && a.amount == was.lanes[0].amount &&
                    a.gesture == was.lanes[0].gesture && a.target == was.lanes[0].target && store.count() == count &&
                    store[page.index()].lanes[0].amount == was.lanes[0].amount && store[page.index()].length == was.length,
                "Shift + encoder 4 turned: nothing; Shift + an encoder's press: nothing");
        require(!page.on_points(), "no the shape page from those");
    }
    // Build 047 (048: a plain tap): a tap of encoder 4 mutes the lane shown, and again unmutes it;
    // the amount reads "muted", the curve draws dim, and the variation is stored with it.
    {
        const u32 count = store.count();
        frame_ok(false);
        require(!a.muted && !d.lanes[1].muted && !std::strcmp(text(screen::t_mark), "delete") && !text(screen::t_name_4)[0],
                "the Shift row: delete over M4 (build 052: the mark slot; column 4 blank)");
        give(press(keys::m1, true));
        require(!a.muted && page.lane_index() == 1, "build 057: Shift + M1 mutes nothing (it switches lane)");
        give(press(keys::m1, true));
        require(page.lane_index() == 2, "Shift + M1 on to lane c (build 060)");
        give(press(keys::m1, true));
        require(page.lane_index() == 0, "Shift + M1 round to lane a");
        give(press(keys::m1 + 1));
        require(a.muted && !d.lanes[1].muted && store.count() == count && store[page.index()].lanes[0].muted &&
                    !store[page.index()].lanes[1].muted && page.stored(),
                "encoder 4's tap: lane a muted, in the store");
        frame_ok(false);
        give(shift_down);
        frame_ok(false);
        require(!std::strcmp(text(screen::t_mark), "delete") && !text(screen::t_name_4)[0] && !text(screen::t_value_4)[0], "muted: the Shift row unchanged");
        PageInput up = shift_down;
        up.kind = PageInput::release, up.shift = false;
        give(up);
        frame_ok(false);
        require(!std::strcmp(text(screen::t_value_4), "muted") && !std::strcmp(text(screen::t_name_4), "amount"), "no Shift: the amount reads muted");
        require(visible(frame.nodes[screen::n_shape]) && frame.nodes[screen::n_shape].shade == screen::s_dim, "its curve in the dim grey");
        give(press(keys::m1));  // lane b: not muted, the amount a percentage again
        frame_ok(false);
        require(text(screen::t_value_4)[0] >= '0' && text(screen::t_value_4)[0] <= '9', "lane b: its own amount, a percentage");
        require(sketch_check::pieces_shown(frame, 1, 0) == 0 || sketch_check::pieces_shade(frame, 1, 0, screen::s_dim), "lane a, behind, dim");
        give(press(keys::m1)), give(press(keys::m1));  // build 060: c, then a
        give(press(keys::m1 + 1));
        give(shift_down);
        frame_ok(false);
        require(!a.muted && !store[page.index()].lanes[0].muted && !std::strcmp(text(screen::t_mark), "delete"), "again: unmuted");
    }
    give(turn(0, 1, true));
    PageInput shift_up = shift_down;
    shift_up.kind = PageInput::release, shift_up.shift = false;
    give(shift_up);
    frame_ok(false);
    require(!page.shift() && !std::strcmp(text(screen::t_name_1), "move") &&
                !std::strcmp(text(screen::t_name_2), "gesture") && !std::strcmp(text(screen::t_name_4), "amount") &&
                !std::strcmp(text(screen::t_value_2), "break") && text(screen::t_value_4)[0],
            "Shift let go: the encoders' layer");
    give(click(1, true));  // any input carries Shift: the layer follows it (Shift + encoder 2's press: nothing)
    frame_ok(false);
    require(page.shift() && !std::strcmp(text(screen::t_name_1), "snap"), "Shift seen on another input");
    give(shift_up);
    // Shift + M4 (build 021, stock's song page delete): the variation leaves
    // the store and the page closes, asking the song page to say deleted. No undo screen.
    require(!page.input(store, press(keys::m1 + 1, true), &names) && !page.input(store, press(keys::m1 + 2, true), &names) &&
                store.count() == 3,
            "Shift + M2 or M3: nothing");
    require(page.input(store, press(keys::m1 + 3, true), &names), "a delete closes the page");
    require(!page.stored() && store.count() == 2 && store[0].offset == 32, "deleted; the others stay");
    const char* said = page.notice();
    require(said && !std::strcmp(said, "deleted") && !page.notice(), "deleted, once, for the song page");
    page.close(store);
    require(store.count() == 2 && store[0].offset == 32 && !page.shift(), "kept on close");

    // An empty lane a (version 2's clear): shown "-", no motion, not deleted on close.
    {
        Store old;
        Draft c = {};
        c.song = 0, c.entry = 1, c.offset = -16, c.length = 16;
        clear_lane(c.lanes[0]), clear_lane(c.lanes[1]), clear_lane(c.lanes[2]);
        c.scene_before = 0, c.scene_after = 1;
        require(old.put(0, c), "an empty variation");
        page.open(old, song_1(1), heard, &names);
        frame_ok();
        require(page.stored() && !std::strcmp(text(screen::t_value_2), "-") &&
                    !visible(frame.nodes[screen::n_shape]) && sketch_check::all_shown(frame) == 0,
                "empty");
        page.close(old);
        require(old.count() == 1, "kept on close");
        page.open(old, song_1(1), heard, &names);
        give_to(old, turn(1, 1));
        require(page.draft().lanes[0].gesture == brk && old[0].lanes[0].gesture == brk && old[0].lanes[0].target == kVolume,
                "a turn: the first gesture, on the volume");
        page.close(old);
    }

    // A join with none: a new break ending on it, default tracks from what was heard; lane b empty.
    heard.note(0, 0, true, 9999), heard.note(0, 1, true, 9999), heard.note(0, 2, false, 9999);
    heard.note(1, 0, true, 9999), heard.note(1, 1, true, 9999), heard.note(1, 3, false, 9999), heard.note(1, 4, false, 9999);
    heard.note(1, 7, false, 100);  // in the scene's last steps: maybe the next scene's, not counted
    require(heard.scene[0] == 0x07 && heard.scene[1] == 0x1b && heard.percussion == 0x03, "heard");
    page.open(store, song_1(0), heard, &names);  // the join at the song's start: scene 2 > scene 1
    require(page.stored() && store.count() == 3 && page.index() == 2, "a new one, stored at once");
    require(a.gesture == brk && d.length == 32 && d.offset == -32 && a.amount == 128 && a.default_tracks, "the default");
    require(a.target == kVolume && a.default_target, "on the volume, following the gesture");
    require(a.selection == 0x18, "break: the tracks leaving (scene 2's 4, 5 not in scene 1)");
    require(b.gesture == kNoGesture && b.target == kNoTarget && !b.selection, "lane b empty");
    frame_ok();
    require(page.fresh() && !std::strcmp(text(screen::t_title), "variation 1 of 3 new"), "header: new, first in the song (its start)");
    require(!std::strcmp(text(screen::t_value_3), "vol") && sketch_check::all_shown(frame) == 0, "its target; nothing behind");
    {
        const char* m = text(screen::t_number_a);  // build 054: the window's start bar, or a message in its place
        require(!page.message() && (!m[0] || m[0] == '-' || (m[0] >= '0' && m[0] <= '9')), "no passing message");
    }
    PageTick much_later = {10 * unit, false};
    page.draw(much_later, frame);
    require(!std::strcmp(text(screen::t_title), "variation 1 of 3 new"), "new stays");
    give(press(keys::plus)), give(press(keys::minus));
    frame_ok(false);
    require(page.fresh() && !std::strcmp(text(screen::t_title), "variation 1 of 3 new") && !std::strcmp(page.message(), "no others"),
            "- and + with none other on this join (no song shape): still new, said");
    // Encoder 1's tap the first time: the empty lane b takes lane a's gesture and amount, the
    // gesture's target, no tracks (nothing plays); still new.
    give(press(keys::m1));
    frame_ok(false);
    require(page.fresh() && b.gesture == brk && b.amount == a.amount && b.target == kVolume && b.default_target && !b.selection &&
                !b.default_tracks && store[2].lanes[1].gesture == brk && store[2].lanes[1].selection == 0,
            "lane b: lane a's break and amount, on the volume, no tracks (stored)");
    require(!std::strcmp(text(screen::t_value_2), "break") && !std::strcmp(text(screen::t_name_3), "tracks") &&
                !std::strcmp(text(screen::t_value_3), "none") && sketch_check::pieces_shown(frame, 1, 0) == screen::sketch_pieces &&
                !visible(frame.nodes[screen::n_shape]),
            "lane b: break, no tracks; lane a's motion dim behind");
    page.leds(leds);
    require(leds.track[3] == led::off && leds.track[4] == led::off, "no keys lit for lane b");
    // Lane b's list starts with none: encoder 2 still empties it, and once
    // emptied it stays so through M1 and back.
    give(turn(1, -1));
    require(b.gesture == kNoGesture && !b.selection && b.target == kNoTarget, "back to none: empty");
    give(press(keys::m1)), give(press(keys::m1)), give(press(keys::m1));  // build 060: c, a, b
    require(page.lane_index() == 1 && b.gesture == kNoGesture, "lane a's gesture is taken only the first time");
    give(turn(1, -1));
    require(b.gesture == kNoGesture, "none is lane b's first");
    // A gesture there takes its default tracks and the cutoff.
    for (u32 i = 0; i < 13; ++i) give(turn(1, 1));  // build 062: hold sits before rise, one more detent
    require(!page.fresh() && b.gesture == rise && b.target == kCutoff && b.default_target && b.default_tracks, "rise on the cutoff");
    require(b.selection == 0x1f && store[2].lanes[1].gesture == rise, "every track heard either side; stored");
    frame_ok(false);
    require(!std::strcmp(text(screen::t_value_3), "cutoff") && !std::strcmp(text(screen::t_title), "variation 1 of 3") && !std::strcmp(text(screen::t_join), "scene 2 > end"),
            "edited: new gone (build 024: the song's start join, a window before it: \"scene 2 > end\")");
    require(visible(frame.nodes[screen::n_shape]) && sketch_check::pieces_shown(frame, 1, 0) == screen::sketch_pieces, "both lanes drawn (lane a sketched behind)");
    // No gesture-level curve (build 020): each gesture bends by its own
    // (store.hpp default_curve), and Shift + encoder 2 is the length, which
    // leaves the curve alone. The Shift row shows no curve.
    {
        PageInput down = press(keys::shift);
        down.shift = true;
        give(down);
        frame_ok(false);
        require(b.curve == kSlowStart && b.points[1].curve == kSlowStart, "the rise's own curve: slow start, its points bent by it");
        for (u32 slot = 0; slot < screen::texts; ++slot)
            require(std::strcmp(text(slot), "curve") && std::strcmp(text(slot), "slow") && std::strcmp(text(slot), "fast"), "no curve in the Shift row");
        const Draft was = d;
        give(turn(1, 1, true));
        require(d.length == 64 && b.curve == kSlowStart && b.points[1].curve == kSlowStart, "Shift + encoder 2: the length, the curve kept");
        give(turn(1, -1, true));
        require(d.length == was.length && d.offset == was.offset && b.curve == kSlowStart, "and back");
        // A gesture change takes that gesture's own curve.
        PageInput up = down;
        up.kind = PageInput::release, up.shift = false;
        give(up);
        give(turn(1, 1));
        require(b.gesture == fall && b.curve == kFastStart && b.points[1].curve == kFastStart, "fall: its own fast start");
        give(turn(1, -1));
        require(b.gesture == rise && b.curve == kSlowStart, "rise again: slow");
        give(turn(1, 2));
        require(b.gesture == swell && b.curve == default_curve(swell), "swell: its own");
        give(turn(1, 1));
        require(b.gesture == dip && b.curve == default_curve(dip), "dip: its own");
        give(turn(1, -3));
        require(b.gesture == rise, "back to the rise");
        // The fades (build 016), just before rise: their own curves. Build 039: any
        // target, the lane's kept (016-038 fixed them to the volume). A target chosen
        // by hand first, so it no longer follows the gesture.
        give(turn(2, 1));
        require(!b.default_target, "chosen by hand");
        const u32 before_fades = b.target;
        give(turn(1, -1));
        // Build 062 ("flat line / silence curve"): hold sits between the fades and rise; any target, kept, no bend.
        require(b.gesture == hold && b.curve == 0 && b.target == before_fades && !b.default_target, "hold: no bend, the target kept");
        give(turn(1, -1));
        require(b.gesture == fade_out && b.curve == kFastStart && b.target == before_fades, "fade out: fast start, the target kept");
        give(turn(2, 1));
        require(b.target != before_fades && store[2].lanes[1].target == b.target, "encoder 3 moves a fade's target");
        give(turn(2, -1));
        require(b.target == before_fades, "and back");
        give(turn(1, -1));
        require(b.gesture == fade_in && b.curve == kSlowStart && b.target == before_fades, "fade in: slow start, the target kept");
        {
            require(b.count == 2 && b.points[0].value == down_by(b.amount) &&
                        b.points[1].value == 128 && b.points[1].curve == kSlowStart,
                    "fade in: from below by the amount to as set");
        }
        give(turn(1, 3));  // build 062: past hold too
        require(b.gesture == rise && b.target == before_fades, "past the fades: the target as it was");
        give(turn(1, -3));
        give(turn(1, -7));
        require(b.gesture == drop, "drop");
        give(turn(1, 10));
        require(b.gesture == rise && b.curve == kSlowStart, "back to the rise");
    }
    // Both lanes heard: lane a still breaks tracks 4 and 5's volume.
    require(store[2].lanes[0].selection == 0x18 && store[2].lanes[0].count == 3, "lane a as it was");
    give(turn(1, -13));
    require(b.gesture == kNoGesture && !b.selection && !store[2].lanes[1].count && store[2].lanes[1].target == kNoTarget,
            "back to none: lane b empty again");
    // Tracks first, then a gesture: the chosen tracks stay.
    give(press(keys::track + 6));
    require(b.selection == 0x40 && !b.default_tracks && b.gesture == kNoGesture, "a track on an empty lane");
    give(turn(1, 3));
    require(b.gesture == drop && b.selection == 0x40 && b.target == kVolume && store[2].lanes[1].count == 3,
            "its own tracks, the volume");
    give(press(keys::m1)), give(press(keys::m1));  // build 060: c, then round to a
    give(turn(1, 1));
    frame_ok(false);
    require(a.selection == 0x04, "build: the tracks arriving (scene 1's 3)");
    give(turn(0, 1)), give(turn(0, 1));
    require(d.offset == 0 && a.selection == 0x03, "inside scene 1, build: its percussion (1, 2)");
    give(turn(1, 1));
    require(a.gesture == drop && a.selection == 0x07, "drop inside: every track playing there");
    give(press(keys::track + 7));
    give(turn(0, -1)), give(turn(0, -1));
    require(a.selection == 0x87 && !a.default_tracks, "chosen tracks no longer follow");
    page.close(store);
    require(store.count() == 3 && store[2].lanes[0].gesture == drop && store[2].lanes[1].gesture == drop, "kept on close");

    // Song playback off: nothing to edit, said so.
    Around off = {};
    page.open(store, off, heard, &names);
    require(!page.showing(), "no song");
    give(turn(1, 1));
    frame_ok();
    require(!std::strcmp(text(screen::t_title), "no song") && shown_nodes() == 0, "no song, nothing else");
    page.leds(leds);
    require(leds.track[7] == led::off, "no keys lit");
    page.close(store);
    require(store.count() == 3, "nothing changed");

    // A store already full: the page shows its draft but says it isn't kept.
    store.clear();
    Draft one = {};
    one.song = 0, one.entry = 0, one.offset = 0, one.length = 8;
    clear_lane(one.lanes[0]), clear_lane(one.lanes[1]), clear_lane(one.lanes[2]);
    while (store.count() < kMaxVariations) require(store.put(store.count(), one), "fill with others");
    page.open(store, song_1(1), heard, &names);
    require(!page.stored() && !std::strcmp(page.message(), "store full"), "full: not kept, and said");
    give(turn(3, 1));
    require(!page.stored() && store.count() == kMaxVariations, "still not");
    frame_ok();
    page.close(store);
    require(store.count() == kMaxVariations, "nothing removed");

    // A project loaded under the page: it stops editing.
    store.clear();
    page.open(store, song_1(1), heard, &names);
    require(page.stored() && store.count() == 1, "new one");
    store.load(vartest, sizeof vartest);
    give(turn(3, 1));
    require(!page.showing() && store.count() == 3, "a load stops the page touching the store");
    page.close(store);
    require(store.count() == 3, "and closing leaves it");

    // Drawing is redone only when something changed.
    page.open(store, song_1(1), heard, &names);
    frame_ok();
    PageFrame before = frame;
    frame.texts[screen::t_number_a].chars[0] = '#';
    PageTick tick = {16, false};
    page.draw(tick, frame);
    require(frame.texts[screen::t_number_a].chars[0] == '#', "no redraw without a change");
    give(turn(3, 1));
    page.draw(tick, frame);
    require(frame.texts[screen::t_number_a].chars[0] != '#' && std::memcmp(&before.nodes, &frame.nodes, sizeof before.nodes) == 0,
            "a change redraws");
    page.close(store);

    // The playhead: a hairline across the plot (build 049: on the cursor's node, plot high) at the song's place while it
    // plays in the view. The view here: 64 steps, -32..32 (the window the last 2 bars of scene 1).
    store.load(vartest, sizeof vartest);
    page.open(store, song_1(1), heard, &names);  // scene 1 > scene 2, the window the last 2 bars of scene 1
    frame_ok();
    const draw::Node& head = frame.nodes[screen::n_cursor];
    require(!visible(head) && !visible(frame.nodes[screen::n_playhead]) && page.playhead() < 0, "not before a pass");
    SongPosition p = {true, 0, 0, 1, 8 * 480, 64 * 480, 0, kNoScene};  // scene 1 (entry 0), step 8: 56 steps before the join
    page.follow(p);
    page.draw(tick, frame);
    require(!visible(head), "56 steps before the join: out of the view");
    p.position = 40 * 480;  // 24 steps before the join: inside the window (the last 32 steps of scene 1)
    page.follow(p);
    page.draw(tick, frame);
    const i32 want = tl_x(-24, -32, 32) / unit * unit;
    require(visible(head) && head.x == want && head.shade == screen::s_white && head.scale == (screen::plot_y1 - screen::plot_y0) * unit / 100 &&
                head.y == screen::tl_cy * unit && !visible(frame.nodes[screen::n_playhead]),
            "inside the window: white, the plot's height (the cursor's 100 px authored)");
    const i32 first_x = head.x;
    p.position = 40 * 480 + 40;  // 0.08 step later: well under a pixel (6.75 px a step)
    page.follow(p);
    frame.nodes[screen::n_join].x = 7;  // a marker: nothing else is redrawn
    page.draw(tick, frame);
    require(head.x == first_x && frame.nodes[screen::n_join].x == 7, "under a pixel: nothing redrawn");
    p.position = 60 * 480;  // 4 steps before the join
    page.follow(p);
    page.draw(tick, frame);
    require(head.x == tl_x(-4, -32, 32) / unit * unit && head.shade == screen::s_white, "still in the window");
    require(frame.nodes[screen::n_join].x == 7, "only the playhead node moved");
    frame_ok();
    p.entry = 1, p.next_entry = 0, p.scene = 1, p.position = 8 * 480;  // scene 2, step 8: past the window, in the view
    page.follow(p);
    page.draw(tick, frame);
    require(visible(head) && head.x == tl_x(8, -32, 32) / unit * unit && head.shade == screen::s_text, "after the window: grey");
    p.position = 64 * 480;  // scene 2's end: past the view
    page.follow(p);
    page.draw(tick, frame);
    require(!visible(head), "past the view: none");
    p.position = 8 * 480;
    p.song = 1;
    page.follow(p);
    page.draw(tick, frame);
    require(!visible(head), "another song: none");
    p.song = 0, p.playing = false;
    page.follow(p);
    page.draw(tick, frame);
    require(!visible(head), "stopped: none");
    // A long section before: the view is still the window's 64 steps, so a place 192 steps before the join is off it.
    Around long_before = song_1(1);
    long_before.steps_before = 256;
    page.close(store);
    page.open(store, long_before, heard, &names);
    frame_ok();
    p = {true, 0, 0, 1, 64 * 480, 256 * 480, 0, kNoScene};  // 192 steps before the join
    page.follow(p);
    page.draw(tick, frame);
    require(!visible(head), "outside the view: none");
    p.position = 232 * 480;  // 24 before
    page.follow(p);
    page.draw(tick, frame);
    require(visible(head) && head.x == tl_x(-24, -32, 32) / unit * unit, "within it: shown");
    // A change redraws everything, the playhead with it.
    give(turn(0, 1));
    frame_ok(false);
    require(visible(head) && head.x == tl_x(-24, -32, 32) / unit * unit, "kept through a redraw (the window moved inside the view)");
    page.close(store);

    // Song order and - and + across joins. Song 2: scenes 1, 2, 1, 3 (steps 64, 64, 32, 128). Two at
    // the join into entry 2, none into entry 3, one into entry 4, one unplaced into entry 1, one in
    // another song. Numbered: entry 2's at -16 (1), entry 2's at +16 (2), entry 4's (3).
    {
        static SongShape shape = {4, {0, 1, 0, 2}, {64, 64, 32, 128}};
        Store s;
        Draft v = {};
        v.song = 1, v.length = 16;
        clear_lane(v.lanes[0]), clear_lane(v.lanes[1]), clear_lane(v.lanes[2]);
        v.lanes[0].gesture = drop, v.lanes[0].amount = 0, v.lanes[0].selection = 1, v.lanes[0].target = kVolume;
        auto add = [&](u32 entry, i32 offset, bool unplaced, u32 song) {
            v.entry = entry, v.offset = offset, v.unplaced = unplaced, v.song = song;
            v.scene_before = shape.scene[(entry + 3) % 4], v.scene_after = shape.scene[entry];
            generate(v);
            require(s.put(s.count(), v), "put");
        };
        add(3, 0, false, 1);    // store 0: entry 4's
        add(1, 16, false, 1);   // store 1: entry 2's second (by where it starts)
        add(0, -16, true, 1);   // store 2: unplaced (join into entry 1)
        add(1, -16, false, 1);  // store 3: entry 2's first
        add(2, 0, false, 0);    // store 4: another song's, at the join into entry 3
        u32 total = 0;
        require(song_number(s, 1, 3, &total) == 1 && total == 3, "entry 2's first (window starts earlier): 1 of 3");
        require(song_number(s, 1, 1, &total) == 2 && song_number(s, 1, 0, &total) == 3, "then entry 2's second, entry 4's");
        require(!song_number(s, 1, 2, &total) && !song_number(s, 1, 4, &total) && !song_number(s, 1, 9, &total),
                "unplaced, another song's, none: no number");
        require(song_number(s, 0, 4, &total) == 1 && total == 1, "the other song counts its own");
        Around a = {};
        a.song = true, a.song_index = 1, a.entries = 4, a.entry = 1, a.scene_before = 0, a.scene_after = 1;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        page.open(s, a, heard, &names);
        require(page.index() == 3 && page.number() == 1 && page.total() == 3 && page.entry() == 1 && !page.moved(),
                "opens on entry 2's first in song order");
        frame_ok();
        require(!std::strcmp(text(screen::t_title), "variation 1 of 3") && !std::strcmp(text(screen::t_join), "scene 1 > scene 2"), "header: var 1 of 3");
        give_to(s, press(keys::m1));
        give_to(s, press(keys::plus));
        require(page.index() == 1 && page.number() == 2 && page.entry() == 1 && !page.message() && !page.moved(), "+: 2, the same join");
        require(page.lane_index() == 1, "the lane shown stays as chosen");
        give_to(s, press(keys::m1)), give_to(s, press(keys::m1));  // build 060: c, then round to a
        give_to(s, press(keys::plus));
        require(page.index() == 0 && page.number() == 3 && page.entry() == 3, "+: 3, on to entry 4 (entry 3 has none of this song's)");
        require(page.moved(), "moved off the join it opened on: the song cursor follows on close");
        frame_ok(false);
        require(!std::strcmp(text(screen::t_title), "variation 3 of 3") && !std::strcmp(text(screen::t_join), "scene 3") && !std::strcmp(text(screen::t_label_a), "scene 1") &&
                    !std::strcmp(text(screen::t_label_b), "scene 3"),
                "header: var 3 of 3, its scenes (the window inside scene 3)");
        require(!page.message(), "no \"entries\" message: the header says which");
        // Entry 4 (32 steps before it, 128 after): the 16 step window on the join in a 32 step view, centred on it.
        require(near_x(frame.nodes[screen::n_join].x, tl_x(0, -16, 16)), "the timeline placed on the new join: -16..16");
        curve_spans(tl_x(0, -16, 16), tl_x(16, -16, 16));
        SongPosition p = {true, 1, 3, 0, 8 * 480, 128 * 480, 2, kNoScene};  // in entry 4, step 8: inside its window
        page.follow(p);
        frame_ok(false);
        require(visible(frame.nodes[screen::n_cursor]) && frame.nodes[screen::n_cursor].shade == screen::s_white,
                "the playhead follows the new join");
        p.entry = 1, p.length = 64 * 480;
        page.follow(p);
        frame_ok(false);
        require(!visible(frame.nodes[screen::n_cursor]), "not in the old join's sections");
        give_to(s, press(keys::plus));
        require(page.index() == 3 && page.number() == 1 && page.entry() == 1 && !page.moved(),
                "+ wraps round the song, past the unplaced one, to 1 (the join it opened on)");
        give_to(s, press(keys::minus));
        require(page.index() == 0 && page.entry() == 3, "- wraps back to 3");
        give_to(s, press(keys::minus));
        require(page.index() == 1 && page.entry() == 1 && page.number() == 2, "-: 2");
        frame_ok(false);
        require(!std::strcmp(text(screen::t_title), "variation 2 of 3") && !std::strcmp(text(screen::t_join), "scene 2"), "its header");
        // Moving a window renumbers: entry 2's second slid level with the first (-1 bar) comes
        // first by store order.
        for (u32 i = 0; i < 2; ++i) give_to(s, turn(0, -1));
        require(page.draft().offset == -16 && page.number() == 1, "slid to -1 bar: now 1");
        frame_ok(false);
        require(!std::strcmp(text(screen::t_title), "variation 1 of 3") && !std::strcmp(text(screen::t_join), "scene 1 > scene 2"), "renumbered in the header");
        give_to(s, press(keys::plus));
        require(page.index() == 3 && page.number() == 2, "+: the other, now 2");
        // An edit after a jump lands on that join, with its scenes.
        give_to(s, press(keys::plus));
        give_to(s, turn(3, 5));
        require(s[0].entry == 3 && s[0].lanes[0].amount > 0 && s[0].scene_before == 0 && s[0].scene_after == 2, "edited at entry 4");
        // Deleting the only one at a join (after a jump): out of the store, the page closes.
        require(page.input(s, press(keys::m1 + 3, true), &names) && s.count() == 4 && !std::strcmp(page.notice(), "deleted"),
                "deleted at entry 4, closing");
        require(page.moved() && page.entry() == 3, "closing on entry 4's join: the song cursor follows there");
        page.close(s);
        require(s.count() == 4, "the deleted one stays deleted");
        // Left: entry 2's two, the unplaced one, the other song's. Reopened: 1 of 2.
        page.open(s, a, heard, &names);
        require(page.number() == 1 && page.total() == 2, "1 of 2");
        give_to(s, press(keys::minus));
        require(page.number() == 2 && page.entry() == 1, "- round to the last, on the same join");
        page.close(s);
        // One alone in its song: - and + say so.
        Store one;
        add(1, 0, false, 1);
        Draft only;
        s.draft(s.count() - 1, only);
        require(one.put(0, only), "one");
        page.open(one, a, heard, &names);
        give_to(one, press(keys::minus));
        require(page.index() == 0 && !std::strcmp(page.message(), "no others"), "-: no others");
        give_to(one, press(keys::plus));
        require(page.index() == 0 && !std::strcmp(page.message(), "no others"), "+: no others");
        page.close(one);
    }

    // Every target's name fits under the gesture.
    for (u32 i = 0; i <= opxy::extensions::track_target_count; ++i) {
        const opxy::extensions::TrackTarget* t = i ? &opxy::extensions::track_targets[i - 1] : nullptr;
        const char* name = t ? t->label : "notes";
        if (name) require(words::width(name, screen::target_size) <= screen::target_width * unit, name);
    }
    require(words::width("param 4", screen::target_size) <= screen::target_width * unit, "param 4");
    // Every message fits the message line (no wider than "entries 1 > 2", which
    // read well; "ONLY THIS JOIN" was squeezed).
    {
        const i32 room = screen::message_width * unit;
        require(words::width("ENTRIES 1 > 2", screen::message_size) <= room &&
                    words::width("ONLY THIS JOIN", screen::message_size) > room,
                "the line's width between what read well and what was squeezed");
        const char* const said[] = {"store full", "no others", "song is off", "no tracks", "points full", "no point", "not in lane"};
        for (const char* m : said) require(words::width(m, screen::message_size) <= room, m);
    }
    // Headers with two-digit numbers and scenes (build 022: the title centred, the scenes top left): both fit, apart.
    {
        Store s;
        Draft v = {};
        v.song = 0, v.entry = 1, v.length = 16, v.offset = -16;
        clear_lane(v.lanes[0]), clear_lane(v.lanes[1]), clear_lane(v.lanes[2]);
        v.lanes[0].gesture = drop, v.lanes[0].selection = 0xff, v.lanes[0].target = kVolume;
        v.scene_before = 98, v.scene_after = 97;
        for (u32 i = 0; i < 12; ++i) {
            generate(v);
            require(s.put(s.count(), v), "put");
        }
        Around a = song_1(1);
        a.scene_before = 98, a.scene_after = 97;
        page.open(s, a, heard, &names);
        frame_ok();
        require(!std::strcmp(text(screen::t_title), "variation 1 of 12") && !std::strcmp(text(screen::t_join), "scene 99 > scene 98"),
                "two-digit scenes: the long form fits left of the centred title");
        for (u32 i = 0; i < 11; ++i) give_to(s, press(keys::plus));
        frame_ok();
        require(!std::strcmp(text(screen::t_title), "variation 12 of 12"), "variation 12 of 12");
        PageLeds leds8 = {};
        page.leds(leds8);
        require(leds8.track[0] == led::on && leds8.track[7] == led::on, "all eight tracks (build 056: their keys, no header list)");
        page.close(s);
    }
    // An amount of 0 draws a flat line across the middle (as set), not at an edge.
    {
        Store s;
        page.open(s, song_1(1), heard, &names);
        give_to(s, turn(1, 12));  // rise // build 062: one detent more, hold is before it
        give_to(s, turn(3, -100, false, 2));
        frame_ok();
        require(page.draft().lanes[0].gesture == rise && page.draft().lanes[0].amount == 0 && page.draft().lanes[0].selection &&
                    !std::strcmp(text(screen::t_value_4), "0%"),
                "rise at 0 %");
        const auto& path = frame.paths[screen::n_shape];
        require(visible(frame.nodes[screen::n_shape]), "drawn");
        for (u32 i = 0; i < screen::points / 2; ++i)
            require(path.points[i].y == path.points[0].y && path.points[i].y > -unit && path.points[i].y < unit,
                    "flat, through the middle");
        page.close(s);
    }
    // Build 075 (photographing one fade+ at 100 % on both screens: "these photos show the same variation, one on the
    // variation screen and the other on the edit screen, but the curve on the edit screen only goes up 50% of the grid"):
    // the variation page draws at true height, as the edit page does. A fade in at full amount runs from silence (0) up to
    // the track's own level (as set, the middle), so it uses the lower half on BOTH pages and never reaches the top.
    {
        using screen::n_shape; using screen::n_shape_b; using screen::plot_y0; using screen::plot_y1; using screen::tl_dy; using screen::points;
        Store s;
        page.open(s, song_1(1), heard, &names);
        for (u32 i = 0; i < 40 && page.draft().lanes[0].gesture != fade_in; ++i) give_to(s, turn(1, 1));
        require(page.draft().lanes[0].gesture == fade_in, "build 075: fade in reached");
        give_to(s, turn(3, 100, false, 2));
        require(page.draft().lanes[0].amount == 255, "build 075: fade in at 100 %");
        // The motion samples themselves: the lower half only, from silence up to as set.
        u32 m[kMotionSamples];
        motion(page.draft().lanes[0], page.draft().length, m);
        u32 lo = 255, hi = 0;
        for (u32 i = 0; i < kMotionSamples; ++i) lo = m[i] < lo ? m[i] : lo, hi = m[i] > hi ? m[i] : hi;
        require(lo == 0 && hi <= kAsSet && hi > kAsSet - 8, "build 075: a fade in at 100 % draws 0 up to as set: the lower half, not the top");
        // The drawn height of the lane's ribbon on each page, as a fraction of the plot (Q16 of its height) from the plot's
        // foot; the variation page's timeline sits tl_dy px below the edit page's plot (build 050), so each is measured
        // from its own foot. Only the top edge of each ribbon (its first half of points) is read.
        auto heights = [&](bool edit, i32* lowest, i32* highest, i32* first, i32* last) {
            frame_ok();
            const long long foot = (plot_y1 + (edit ? 0 : tl_dy)) * unit, plot_h = (plot_y1 - plot_y0) * unit;
            // The variation page's curve is n_shape; the edit page's two halves are paths 0 and 1 (each its own node).
            const u32 ids[2] = {edit ? 0u : static_cast<u32>(n_shape), edit ? 1u : static_cast<u32>(n_shape_b)};
            i32 top = -1000000, bottom = 1000000, f = 0, l = 0;
            bool any = false;
            for (u32 k = 0; k < 2; ++k) {
                if (!visible(frame.nodes[ids[k]])) continue;
                for (u32 i = 0; i < points / 2; ++i) {
                    const long long y = frame.nodes[ids[k]].y + frame.paths[ids[k]].points[i].y * 2;
                    const i32 h = static_cast<i32>((foot - y) * 65536 / plot_h);  // Q16 of the plot, up from its foot
                    top = h > top ? h : top, bottom = h < bottom ? h : bottom;
                    if (!any) f = h, any = true;
                    l = h;
                }
            }
            *highest = top, *lowest = bottom, *first = f, *last = l;
        };
        i32 v_low, v_high, v_first, v_last, e_low, e_high, e_first, e_last;
        heights(false, &v_low, &v_high, &v_first, &v_last);
        require(v_high > 0 && v_high <= 65536 / 2 + 65536 / 40, "build 075: the variation page's fade+ reaches the middle, not the top");
        require(v_low >= -65536 / 40 && v_low <= 65536 / 40, "build 075: and starts on the bottom");
        give_to(s, press(keys::m1 + 2));
        require(page.on_points(), "build 075: on the edit page");
        heights(true, &e_low, &e_high, &e_first, &e_last);
        require(e_high > 0 && e_high <= 65536 / 2 + 65536 / 40, "build 075: the edit page's fade+ reaches the middle too");
        // The point of the build: the same lane, the same height on both screens (within a pixel and a half of the plot).
        const i32 tol = 65536 * 3 / (2 * (plot_y1 - plot_y0));
        auto close = [&](i32 a, i32 b) { return a - b <= tol && b - a <= tol; };
        require(close(v_high, e_high) && close(v_low, e_low) && close(v_first, e_first) && close(v_last, e_last),
                "build 075: the variation page's curve and the edit page's agree in height for the same lane");
        give_to(s, press(keys::m1 + 2));
        require(!page.on_points(), "build 075: back on the variation page");
        // The other direction: a fade out at 100 % (as set down to silence) is the lower half too; a swell, as set up to
        // the top, the upper half; a line over both sides still spans what it has.
        DraftLane l;
        clear_lane(l);
        l.gesture = swell, l.amount = 255, l.selection = 0x01, l.curve = default_curve(swell);
        generate(l, 32);
        motion(l, 32, m);
        lo = 255, hi = 0;
        for (u32 i = 0; i < kMotionSamples; ++i) lo = m[i] < lo ? m[i] : lo, hi = m[i] > hi ? m[i] : hi;
        require(lo >= kAsSet - 8 && hi > 250, "build 075: a swell at 100 % draws as set up to the top: the upper half");
        l.gesture = fade_out, l.curve = default_curve(fade_out);
        generate(l, 32);
        motion(l, 32, m);
        lo = 255, hi = 0;
        for (u32 i = 0; i < kMotionSamples; ++i) lo = m[i] < lo ? m[i] : lo, hi = m[i] > hi ? m[i] : hi;
        require(lo == 0 && hi <= kAsSet && hi > kAsSet - 8, "build 075: a fade out at 100 % draws as set down to silence: the lower half");
        clear_lane(l);
        l.gesture = swell, l.amount = 255, l.selection = 0x01, l.count = 2;
        l.points[0] = {0, 0, 0}, l.points[1] = {31, 255, 0};
        motion(l, 32, m);
        require(m[0] == 0 && m[kMotionSamples - 1] == 255, "build 075: a line over both sides still spans the whole chart");
        l.count = 3;
        l.points[0] = {0, 100, 0}, l.points[1] = {16, 200, 0}, l.points[2] = {31, 100, 0};
        motion(l, 32, m);
        require(m[0] == 100 && m[kMotionSamples - 1] == 100 && m[kMotionSamples / 2] >= 190, "build 075: a line over as set draws at its own heights, not stretched");
        l.count = 1, l.points[0] = {0, static_cast<u8>(kAsSet), 0};
        motion(l, 32, m);
        require(m[0] == kAsSet && m[kMotionSamples / 2] == kAsSet && m[kMotionSamples - 1] == kAsSet,
                "build 075: a flat line at as set still draws across the middle");
        page.close(s);
    }
    // Build 024: the width estimate's fit (width = table + 3/4 px a glyph - 4 px; 30 px and up
    // the table x 1.2): "variation 1 of 5" where 023 had it, 5-glyph words 2.75 px narrower
    // (they move 1.4 px right), the 30 px gesture wider (it moves left).
    require(words::estimate("variation 1 of 5", 12) == 93102, "the long title: as 023");
    require(words::estimate("shape", 12) == 37089 - 2816 && words::estimate("notes", 12) == 34385 - 2816, "short words narrower");
    // Build 025: 30 px and up the table x 1.1, the midpoint of 023's 1.0 (the gesture right of
    // centre) and 024's 1.2 (left of it): "break" 88616, halfway between 80537 and 96696.
    require(words::estimate("break", 30) == 88616 && 2 * 88616 - 80537 - 96696 <= 1, "the 30 px gesture: the midpoint");
    require(words::estimate("variation 1 of 5", 12) == 93102, "12 px texts unchanged by the 30 px scale");
    require(words::width("", 12) == 0 && words::width("-", 10) > 0, "nothing is 0 wide, a dash is not");
    // Build 026: off the device, without fake metrics, words::width is the estimate (tests/metrics.cpp
    // checks the page with a measure).
    draw::host_text_measure() = nullptr;
    require(words::width("variation 1 of 5", 12) == words::estimate("variation 1 of 5", 12), "no measure: the estimate");
    draw::host_text_measure() = texts_check::device_like;
    // Every name the SDK's track targets give fits the shape page's target place whole (023 cut "env amount" to "env
    // amou" beside its place in the list). Build 052: and the control strip's value, 15 px in 55 px, by its short
    // name (words::short_target: the names over the room are shortened as chosen, nothing cut by words::fit).
    {
        // Build 053: the shortening is wider and harder, so a value leaves its cell air instead of filling it.
        const char* const longer[][2] = {{"attack", "atk"}, {"decay", "dec"}, {"sustain", "sus"}, {"release", "rel"},
                                          {"volume", "vol"}, {"resonance", "reso"}, {"env amount", "env"}, {"key track", "key trk"},
                                          {"lfo speed", "lfo spd"}, {"lfo amount", "lfo amt"}, {"filter attack", "flt atk"},
                                          {"filter decay", "flt dec"}, {"filter sustain", "flt sus"}, {"filter release", "flt rel"},
                                          {"ext send", "ext"}, {"tape send", "tape"}, {"fx i send", "fx i"}, {"fx ii send", "fx ii"}};
        u32 shortened = 0;
        for (u32 i = 0; i < opxy::extensions::track_target_count; ++i) {
            const char* label = opxy::extensions::track_targets[i].label;
            if (!label) continue;
            char fitted[draw::text_capacity], shown[draw::text_capacity];
            require(words::width(label, screen::target_size) <= screen::target_width * unit, label);
            words::short_target(shown, label);
            for (const auto& pair : longer)
                if (!std::strcmp(label, pair[0])) require(!std::strcmp(shown, pair[1]), pair[0]), ++shortened;
            words::fit(fitted, shown, screen::strip_room * unit, screen::value_size);
            require(!std::strcmp(fitted, shown), shown);  // whole at 15 px in 55 px
            if (!std::strcmp(shown, label)) require(words::width(label, screen::value_size) <= screen::strip_room * unit, label);
        }
        require(shortened == 18, "all eighteen shortened names, no others");
        // Build 053: every strip value leaves at least 9 px of its 55 px cell, so it does not read as crowded
        // (on 052: "random, wander, env amt, decay !, volume, +n bars, on join" at 44 to 53 px "don't fit").
        // The two widest left are "key trk" (45) and "lfo amt" (46): "track" and "amount" have no shorter form that
        // still reads, and dropping their space ("keytrk", "lfoamt") was worse. Every name chosen is 44 or under.
        for (u32 i = 0; i < opxy::extensions::track_target_count; ++i) {
            const char* label = opxy::extensions::track_targets[i].label;
            if (!label) continue;
            char shown_i[draw::text_capacity];
            words::short_target(shown_i, label);
            if (words::width(shown_i, screen::value_size) > (screen::strip_room - 9) * unit) {
                std::fprintf(stderr, "%s -> %s: %d px\n", label, shown_i, words::width(shown_i, screen::value_size) / unit);
                require(false, "a target name with 10 px of its cell to spare");
            }
        }
        // Build 052: every gesture's name fits the strip's value (15 px, 55 px); four are the page's short forms.
        for (u32 g = 0; g < kGestureCount; ++g)
            require(words::width(kGestureNames[g], screen::value_size) <= screen::strip_room * unit, kGestureNames[g]);
        // Build 053: no space before a sign, and the longest names shortened again.
        require(!std::strcmp(kGestureNames[fade_in], "fade+") && !std::strcmp(kGestureNames[fade_out], "fade-") &&
                    !std::strcmp(kGestureNames[stutter_up], "stut+") && !std::strcmp(kGestureNames[stutter_down], "stut-") &&
                    !std::strcmp(kGestureNames[stutter], "stut") && !std::strcmp(kGestureNames[random_], "rand") &&
                    !std::strcmp(kGestureNames[wander], "wand") && !std::strcmp(kGestureNames[custom], "edit"),
                "the gesture names the page shortens");
        // Build 062 ("flat line / silence curve"): "hold", not "flat" (the curve column's word for no bend), and it
        // fits the strip's cell by the device's measure, with the air 053 asked for.
        require(!std::strcmp(kGestureNames[hold], "hold") && words::width(kGestureNames[hold], screen::value_size) <= screen::strip_room * unit &&
                    texts_check::device_like(screen::value_size, kGestureNames[hold]) <= 2 * screen::strip_room / 3,
                "build 062: hold reads \"hold\" within its cell");
        for (u32 g = 0; g < kGestureCount; ++g)
            require(words::width(kGestureNames[g], screen::value_size) <= (screen::strip_room - 9) * unit, kGestureNames[g]);
        // Build 052: build 039's warning (" !", 13 px at 15 px) has to survive the strip's 55 px. Every name that can
        // carry it (the SDK's targets 9-11 and the engine controls costly() names) fits whole once shortened: without
        // that, words::fit drops the "!" and keeps the word, which loses the signal altogether.
        char shown[draw::text_capacity], warned_fit[draw::text_capacity];
        static const char* const warned[] = {"decay !", "sustain !", "release !", "length !", "hold !"};
        for (const char* warn : warned) {
            words::short_target(shown, warn);
            words::fit(warned_fit, shown, screen::strip_room * unit, screen::value_size);
            const u32 len = words::length(shown);
            require(!std::strcmp(warned_fit, shown) && len > 2 && shown[len - 1] == '!' && shown[len - 2] == ' ', warn);
        }
        words::short_target(shown, "release !");
        require(!std::strcmp(shown, "rel !"), "the envelope's words are their three letter forms, warning and all");
        words::short_target(shown, "decay !");
        require(!std::strcmp(shown, "dec !"), "and decay");
        words::short_target(shown, "length !");
        require(!std::strcmp(shown, "len !"), "an engine control named length too (54 px whole with the warning)");
        words::short_target(shown, "filter release !");
        require(!std::strcmp(shown, "flt rel !"), "the filter's keep flt in front");
        words::short_target(shown, "param 1");
        require(!std::strcmp(shown, "par 1"), "build 053: the engine's M1 slots (54 px whole)");
        words::short_target(shown, "tone");
        require(!std::strcmp(shown, "tone"), "a name that fits is left alone");
        // Build 059 ("somehow the word random has found it's way in again, and is too long, it should be rand
        // probably"): an engine control named "random" or "wander" reaches the strip unshortened, and though it fits
        // the cell it fills it. They take the gesture list's own short forms, so a word means one thing on the page.
        words::short_target(shown, "random");
        require(!std::strcmp(shown, "rand"), "build 059: random is rand, as the gesture list has it");
        words::short_target(shown, "wander");
        require(!std::strcmp(shown, "wand"), "build 059: wander is wand");
        require(!std::strcmp(shown, kGestureNames[wander]), "and the same word the gesture uses");
        words::short_target(shown, "random !");
        require(!std::strcmp(shown, "rand !"), "the CPU warning still fits beside it");
        // The crowding rule 053 set: a shortened value takes about two thirds of the 55 px cell, not all of it.
        words::short_target(shown, "random");
        require(texts_check::device_like(screen::value_size, shown) <= 2 * screen::strip_room / 3,
                "build 059: and it leaves the cell air, as 053 asked");
    }
    {
        char fitted[draw::text_capacity];
        words::fit(fitted, "filter release env", 80 * unit, screen::small_size);
        require(!std::strcmp(fitted, "filter release"), "too long: its first words that fit");
        words::fit(fitted, "abcdefghijklmnopqrstuvw", 40 * unit, screen::small_size);
        require(fitted[0] && words::width(fitted, screen::small_size) <= 40 * unit, "one long word: cut to fit");
        // Build 059 ("can we add an elipsis or something to cover ourselves"): a word cut mid-way says so, and
        // the mark is inside the room, not past it. Dropping whole trailing words is not a cut and is not marked.
        {
            const u32 n = words::length(fitted);
            require(n > 3 && !std::strcmp(fitted + n - 3, "..."), "build 059: a cut word is marked");
        }
        words::fit(fitted, "filter release env", 80 * unit, screen::small_size);
        require(!std::strcmp(fitted, "filter release"), "build 059: fewer whole words is not a cut, so no mark");
        // An engine could name a control anything: whatever it is, the result stays inside the cell, and says it was
        // cut whenever a word was broken. Sweep lengths to prove both at the strip's own room and size.
        for (u32 len = 1; len <= 23; ++len) {
            char word[24];
            for (u32 k = 0; k < len; ++k) word[k] = static_cast<char>('a' + k % 26);
            word[len] = 0;
            words::fit(fitted, word, screen::strip_room * unit, screen::value_size);
            require(fitted[0] && words::width(fitted, screen::value_size) <= screen::strip_room * unit,
                    "build 059: any single word fits its cell");
            const u32 n = words::length(fitted);
            const bool cut = n < len || std::strcmp(fitted, word) != 0;
            if (cut) require(n > 3 && !std::strcmp(fitted + n - 3, "..."), "build 059: and says so when cut");
        }
    }
    // Engine controls on a lane of mixed engines (build 023 on hardware: every track on the
    // lane, track 1 a drum engine whose slots stock leaves unnamed; track 3 a synth with "tone,
    // env, shape, tremolo"). Build 025: param 1..4 always, then every engine's names, each once,
    // the lowest track's first; unsafe names and slots left out.
    {
        struct Fake {
            static u32& kind() { static u32 k = 0; return k; }  // 0 drum + synth, 1 two synths, 2 unreadable, 3 sampler + synth
            static bool name(u32 track, u32 word, char* text, u32 capacity) {
                const opxy::extensions::TrackTarget* t = opxy::extensions::track_target(word);
                if (!t || kind() == 2) return text[0] = 0, false;
                const char* n = t->label;
                if (!n) {
                    static const char* const synth[8] = {"tone", "env", "shape", "tremolo", nullptr, nullptr, nullptr, nullptr};
                    static const char* const other[8] = {"pitch", "env", "tone", nullptr, nullptr, nullptr, nullptr, nullptr};
                    // build 034: as the SDK gives a drum's slots, unnamed ("m1 N"); this one has no 4th
                    static const char* const drum[8] = {"m1 1", "m1 2", "m1 3", nullptr, nullptr, nullptr, nullptr, nullptr};
                    static const char* const sampler[8] = {"start", "tune", "env", "end", "loop", nullptr, nullptr, nullptr};
                    n = track == 0 ? (kind() == 1 ? other[word] : kind() == 3 ? sampler[word] : drum[word]) : synth[word];
                    if (!n) return text[0] = 0, false;
                }
                std::strncpy(text, n, capacity - 1);
                text[capacity - 1] = 0;
                return true;
            }
        };
        static const Names fake = {Fake::name};
        auto list = [&](u32 kind, const char* const* want, u32 count) {
            Fake::kind() = kind;
            Store s;
            Draft v = {};
            v.song = 0, v.entry = 1, v.offset = -32, v.length = 32, v.scene_before = 0, v.scene_after = 1;
            clear_lane(v.lanes[0]), clear_lane(v.lanes[1]), clear_lane(v.lanes[2]);
            v.lanes[0].gesture = rise, v.lanes[0].amount = 128, v.lanes[0].target = kTargetNotes, v.lanes[0].selection = 0x05;
            v.lanes[0].curve = default_curve(rise);
            generate(v);
            require(s.put(0, v), "put");
            VariationPage p;
            p.open(s, song_1(1), heard, &fake);
            for (u32 k = 0; k < count; ++k) {
                p.input(s, turn(2, 1), &fake);
                require(!std::strcmp(p.target_text(), want[k]), want[k]);
            }
            require(p.draft().lanes[0].target == 8, "then the amp envelope");
        };
        // Build 032: a lane of two tracks lists "all": the slots and the fixed targets, no
        // engine's own names (025-031 listed them here); param 5 where a lane track has slot 5.
        const char* const drum_synth[] = {"param 1", "param 2", "param 3", "param 4", "attack"};
        list(0, drum_synth, 5);
        list(1, drum_synth, 5);
        list(2, drum_synth, 5);  // no name readable: the params alone
        const char* const sampler_synth[] = {"param 1", "param 2", "param 3", "param 4", "param 5", "attack"};
        list(3, sampler_synth, 6);  // the sampler's slot 5 ("loop") puts param 5 in
        // Build 030: the lane's tracks the target misses, in brackets top right and dim on their keys:
        // "tone" by name is the synth's (track 3), not the drum's (track 1); a slot both lack, both.
        {
            Fake::kind() = 0;
            Store s;
            Draft v = {};
            v.song = 0, v.entry = 1, v.offset = -32, v.length = 32, v.scene_before = 0, v.scene_after = 1;
            clear_lane(v.lanes[0]), clear_lane(v.lanes[1]), clear_lane(v.lanes[2]);
            v.lanes[0].gesture = rise, v.lanes[0].amount = 128, v.lanes[0].target = kTargetNotes, v.lanes[0].selection = 0x05;
            v.lanes[0].curve = default_curve(rise);
            generate(v);
            require(s.put(0, v), "put");
            VariationPage p;
            p.open(s, song_1(1), heard, &fake);
            PageFrame f = {};  // zeroed: the page writes no header, and draw::valid checks those strings
            // Build 056: the header's "1 3" / "(1) 3" is gone; the LEDs say the same (on, or dim: the target misses it),
            // Build 057 ("the * doesn't fit"): and the strip's target value carries no mark for a missed track.
            PageLeds leds = {};
            p.draw(PageTick{16, true}, f);
            p.leds(leds);
            require(leds.track[0] == led::on && leds.track[2] == led::on && std::strchr(f.texts[screen::t_value_3].chars, '*') == nullptr,
                    "the notes: both tracks, no mark");
            for (u32 k = 0; k < 4; ++k) p.input(s, turn(2, 1), &fake);
            p.draw(PageTick{16, true}, f);
            p.leds(leds);
            {
                const char* v = f.texts[screen::t_value_3].chars;
                const u32 vn = static_cast<u32>(std::strlen(v));
                require(!std::strcmp(p.target_text(), "param 4") && vn > 0 && !std::strchr(v, '*') && !std::strcmp(v, "par 4") &&
                            words::width(v, screen::value_size) <= screen::strip_room * unit,
                        "param 4: not on the drum, the target value unmarked (build 057: no \" *\"), within its room");
            }
            require(leds.track[0] == led::dim && leds.track[2] == led::on, "its key dim, the synth's lit");
            // Build 032: hold encoder 3 and press track 3: the lane is track 3 alone, its own
            // names listed, "track 3" over the target; a tap of encoder 3 gives both back.
            PageInput hold = {};
            hold.kind = PageInput::held, hold.encoder = 2, hold.key = keys::encoder + 2, hold.encoders = 4;
            p.input(s, hold, &fake);
            PageInput key = press(keys::track + 2);
            key.encoders = 4;
            p.input(s, key, &fake);
            PageInput click = {};
            click.kind = PageInput::click, click.encoder = 2, click.key = keys::encoder + 2;
            p.input(s, click, &fake);  // the hold's own release: nothing
            require(p.draft().lanes[0].selection == 0x04, "track 3 alone");
            p.draw(PageTick{16, true}, f);
            require(!std::strcmp(f.texts[screen::t_name_3].chars, "track 3"), "track 3 over the target");
            for (u32 k = 0; k < 30 && std::strcmp(p.target_text(), "tone"); ++k) p.input(s, turn(2, 1), &fake);
            require(!std::strcmp(p.target_text(), "tone"), "its own: tone");
            p.input(s, click, &fake);  // a plain tap: back to all
            p.draw(PageTick{16, true}, f);
            p.leds(leds);
            require(p.draft().lanes[0].selection == 0x05 && leds.track[0] == led::dim && leds.track[2] == led::on, "both tracks back, the drum missed");
            p.draw(PageTick{16, true}, f);
            require(!std::strcmp(f.texts[screen::t_name_3].chars, "target"), "target again");
            p.input(s, hold, &fake), p.input(s, key, &fake), p.input(s, click, &fake);
            require(p.draft().lanes[0].selection == 0x04, "track 3 again");
            p.input(s, hold, &fake), p.input(s, key, &fake), p.input(s, click, &fake);
            require(p.draft().lanes[0].selection == 0x05, "the same key again: back to both");
        }
  // the sampler's parameter 4, "end", is not offered; its slot 5 "loop" is,
                                     // and (build 030) "param 5" by position, as some lane track has it
    }
    // The song's start join (build 024): "start > scene 1" at or after it, "scene 2 > end"
    // before it, "end > start" across it.
    {
        Store s;
        VariationPage p;
        p.open(s, song_1(0), heard, &names);
        PageFrame f = {};  // zeroed: the page writes no header, and draw::valid checks those strings
        auto lane_text = [&]() {
            p.draw(PageTick{16, true}, f);
            return f.texts[screen::t_join].chars;
        };
        require(p.draft().offset == -32 && !std::strcmp(lane_text(), "scene 2 > end"), "a window before it: scene 2 > end");
        p.input(s, turn(0, 1), &names);
        require(p.draft().offset == -16 && !std::strcmp(lane_text(), "end > start"), "across it: end > start");
        p.input(s, turn(0, 1), &names);
        require(p.draft().offset == 0 && !std::strcmp(lane_text(), "start > scene 1"), "at it: start > scene 1");
        require(!std::strcmp(f.texts[screen::t_label_a].chars, "scene 2, end") && !std::strcmp(f.texts[screen::t_label_b].chars, "start, scene 1"),
                "the ruler's sections: where the song ends and starts");
    }
    // Build 025: a song that doesn't loop (loop off: it stops at its end). On the join into
    // entry 1 the window stays on its side: from the song's start it can't move before the
    // first bar; from the song's end not past the last entry; the header names only that side.
    {
        PageFrame f = {};  // zeroed: the page writes no header, and draw::valid checks those strings
        auto lane_text = [&](VariationPage& p) {
            p.draw(PageTick{16, true}, f);
            return f.texts[screen::t_join].chars;
        };
        {
            Store s;
            VariationPage p;
            Around a = song_1(0);
            a.stops = true;  // opened on the song's start (cursor 0)
            p.open(s, a, heard, &names);
            require(p.draft().offset == 0 && !std::strcmp(lane_text(p), "start > scene 1"), "a new one at the start: from bar 1");
            require(p.at_song_start(), "the song page's cursor goes back to the start");
            p.input(s, turn(0, -1), &names);
            p.input(s, turn(0, -4), &names);
            require(p.draft().offset == 0 && !std::strcmp(lane_text(p), "start > scene 1"), "not before the song's first bar");
            p.input(s, turn(0, 1), &names);
            require(p.draft().offset == 16, "on into the song");
            p.input(s, turn(0, 9), &names);
            require(p.draft().offset == 64 - 32, "to the first entry's end, as before");
            p.input(s, turn(0, -9), &names);
            require(p.draft().offset == 0, "back to bar 1, and no further");
            p.input(s, turn(1, 1, true), &names);  // Shift + encoder 2: 4 bars, keeping its end, but not before the start
            require(p.draft().length == 64 && p.draft().offset == 0, "longer: still from the start");
            p.input(s, turn(1, -2, true), &names);
            require(p.draft().length == 16 && p.draft().offset == 48, "shorter: its end kept");
            p.close(s);
            // Loop on: as 024, across the loop point.
            Store s2;
            VariationPage q;
            q.open(s2, song_1(0), heard, &names);
            q.input(s2, turn(0, 1), &names);
            require(q.draft().offset == -16 && !std::strcmp(lane_text(q), "end > start") && !q.at_song_start(), "looping: across");
        }
        {
            Store s;
            VariationPage p;
            Around a = song_1(0);
            a.stops = true, a.at_end = true;  // opened on the song's end (cursor past the last entry)
            p.open(s, a, heard, &names);
            require(p.draft().offset == -32 && !std::strcmp(lane_text(p), "scene 2 > end"), "a new one at the end: ending on it");
            require(!p.at_song_start(), "the cursor stays at the end");
            p.input(s, turn(0, 1), &names);
            p.input(s, turn(0, 3), &names);
            require(p.draft().offset == -32 && !std::strcmp(lane_text(p), "scene 2 > end"), "not past the song's end");
            p.input(s, turn(0, -2), &names);
            require(p.draft().offset == -64, "back into the last entry, to its start");
            p.input(s, turn(0, -1), &names);
            require(p.draft().offset == -64, "and no further");
            p.input(s, turn(1, -1, true), &names);  // 1 bar, keeping its end
            require(p.draft().length == 16 && p.draft().offset == -48, "shorter: its end kept");
            p.input(s, turn(1, 2, true), &names);
            require(p.draft().length == 64 && p.draft().offset == -64, "longer: still within the last entry");
        }
        {
            // A window across the song's start (made while the song looped): build 030, on a song
            // that doesn't loop it is on the start side and either turn puts it on bar 1 (025-029
            // kept it on the end side, where it could never reach the join; 2026-11-23).
            Store s;
            Draft v = {};
            v.song = 0, v.entry = 0, v.offset = -16, v.length = 32, v.scene_before = 1, v.scene_after = 0;
            clear_lane(v.lanes[0]), clear_lane(v.lanes[1]), clear_lane(v.lanes[2]);
            v.lanes[0].gesture = brk, v.lanes[0].amount = 128, v.lanes[0].target = kVolume, v.lanes[0].selection = 1;
            generate(v);
            require(s.put(0, v), "put");
            VariationPage p;
            Around a = song_1(0);
            a.stops = true;
            p.open(s, a, heard, &names);
            require(p.draft().offset == -16 && !std::strcmp(lane_text(p), "start > scene 1"), "across: shown as the start");
            require(p.at_song_start(), "the cursor goes to the song's start");
            p.input(s, turn(0, -1), &names);
            require(p.draft().offset == 0 && !std::strcmp(lane_text(p), "start > scene 1"), "either turn: onto bar 1");
            p.input(s, turn(0, -1), &names);
            require(p.draft().offset == 0, "then held there");
            p.input(s, turn(0, 1), &names);
            require(p.draft().offset == 16, "and on from it");
        }
        {
            // Build 032 (2026-11-23): put in the last scene while the song looped, then loop
            // off. Seen from the song's start it is not stuck: a turn brings it onto bar 1, and the
            // guard holds from there.
            Store s;
            Draft v = {};
            v.song = 0, v.entry = 0, v.offset = -48, v.length = 32, v.scene_before = 1, v.scene_after = 0;
            clear_lane(v.lanes[0]), clear_lane(v.lanes[1]), clear_lane(v.lanes[2]);
            v.lanes[0].gesture = brk, v.lanes[0].amount = 128, v.lanes[0].target = kVolume, v.lanes[0].selection = 1;
            generate(v);
            require(s.put(0, v), "put");
            VariationPage p;
            Around a = song_1(0);
            a.stops = true;
            p.open(s, a, heard, &names);
            require(p.draft().offset == -48, "shown where it was");
            p.input(s, turn(0, -1), &names);
            require(p.draft().offset == 0 && !std::strcmp(lane_text(p), "start > scene 1"), "a turn: onto bar 1");
            p.input(s, turn(0, -1), &names);
            require(p.draft().offset == 0, "and not back before it");
        }
        {
            // Other joins of a song that doesn't loop: as before.
            Store s;
            VariationPage p;
            Around a = song_1(1);
            a.stops = true;
            p.open(s, a, heard, &names);
            p.input(s, turn(0, 1), &names);
            require(p.draft().offset == -16 && !std::strcmp(lane_text(p), "scene 1 > scene 2"), "another join: across it");
        }
    }
    // Build 046: the variation page's curve is the line as playback reads it, at
    // the lane's rate: a swell at 4x draws four humps, the gate its on/off every beat.
    {
        DraftLane l;
        clear_lane(l);
        l.gesture = swell, l.amount = 255, l.selection = 0x01, l.curve = default_curve(swell);
        generate(l, 32);
        u32 one[kMotionSamples], four[kMotionSamples];
        motion(l, 32, one);
        l.rate = 4;
        motion(l, 32, four);
        const u32 n = kMotionSamples;
        // Build 075: motion() no longer doubles a one-sided line, so the swell (as set up to 255) draws in the top
        // half at its true height: every threshold below is the old one halved above kAsSet (v -> kAsSet + v / 2).
        // Changed assertions: "rate 1: one hump", "rate 4: four humps", "rate 4: down between the humps".
        const u32 H = kAsSet;
        require(one[0] < H + 8 && one[n / 2] > H + 100 && one[n / 8] < H + (one[n / 2] - H) / 2, "rate 1: one hump, peaking in the middle");
        require(four[0] < H + 8 && four[n / 8] > H + 100 && four[3 * n / 8] > H + 100 && four[5 * n / 8] > H + 100 && four[7 * n / 8] > H + 100,
                "rate 4: four humps");
        require(four[n / 4] < H + 50 && four[n / 2] < H + 50, "rate 4: down between the humps");
        bool differs = false;
        for (u32 i = 0; i < n; ++i) differs |= one[i] != four[i];
        require(differs, "the drawn path differs between rate 1 and 4");
        // The gate on 32 steps: on a beat (4 steps), off a beat, at rate 1 as stored once a bar... drawn as played.
        clear_lane(l);
        l.gesture = gate, l.amount = 255, l.selection = 0x01;
        generate(l, 32);
        u32 g[kMotionSamples];
        motion(l, 32, g);
        // Build 075: changed assertion: the gate swings between 0 and as set, drawn at true height (see the swell's note above).
        require(g[1] > kAsSet - 28 && g[n * 5 / 32] < 16 && g[n * 9 / 32] > kAsSet - 28 && g[n * 13 / 32] < 16, "the gate: on and off every beat");
        // Build 048: at 8x on 8 bars (a cycle every step, 128 across 32 samples) the samples all
        // fell on one phase and the gate drew as one hump. Now a zigzag between on and off all along.
        generate(l, 128);
        l.rate = 8;
        motion(l, 128, g);
        u32 swings = 0;
        for (u32 i = 1; i < n; ++i) swings += (g[i] > kAsSet - 28 && g[i - 1] < 16) || (g[i] < 16 && g[i - 1] > kAsSet - 28);
        require(swings >= n - 2, "the gate at 8x: drawn as a chop all along");
    }
    // Build 064 (on hold: "why is it showing only 50% down the screen, at 100% I'd expect it flat along the bottom
    // to indicate nothing"): a flat line draws where its value is; only a line all at as set draws across the middle.
    {
        DraftLane l;
        clear_lane(l);
        l.gesture = swell, l.amount = 255, l.selection = 0x01, l.curve = default_curve(swell);
        u32 m[kMotionSamples];
        const struct { u8 v; u32 lo, hi; const char* what; } flats[] = {
            {0, 0, 0, "build 064: a flat 0 draws along the bottom"},
            {255, 250, 255, "build 064: a flat 255 draws near the top"},
            {static_cast<u8>(kAsSet), kAsSet, kAsSet, "build 064: a flat line at as set draws across the middle"}};
        for (const auto& f : flats) {
            l.count = 1, l.points[0] = {0, f.v, 0};
            motion(l, 32, m);
            bool ok = true;
            for (u32 i = 0; i < kMotionSamples; ++i) ok = ok && m[i] >= f.lo && m[i] <= f.hi;
            require(ok, f.what);
        }
        l.count = 0;  // no points: as set, the middle as before
        motion(l, 32, m);
        require(m[0] == kAsSet && m[kMotionSamples - 1] == kAsSet, "build 064: no points still draws across the middle");
        clear_lane(l);
        l.gesture = hold, l.amount = 255, l.selection = 0x01, l.curve = default_curve(hold);
        generate(l, 32);
        motion(l, 32, m);
        require(m[0] == 0 && m[kMotionSamples / 2] == 0, "build 064: hold at full amount draws along the bottom (The reported case)");
        l.amount = 0;
        generate(l, 32);
        motion(l, 32, m);
        require(m[0] == kAsSet && m[kMotionSamples - 1] == kAsSet, "build 064: hold at amount 0 is as set: the middle");
    }
    // Build 047 (long form): a window that runs on past its section into the entries after it. Song: 5 entries,
    // scenes 1-5 of 64, 64, 64, 32 and 32 steps; the page on the join into entry 2 (index 1).
    {
        static SongShape shape = {5, {0, 1, 2, 3, 4}, {64, 64, 64, 32, 32}};
        Store s;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 5, a.entry = 1, a.scene_before = 0, a.scene_after = 1;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        p.open(s, a, heard, &names);
        PageFrame f = {};  // zeroed: the page writes no header, and draw::valid checks those strings
        auto draw = [&]() { p.draw(PageTick{16, true}, f); };
        require(p.draft().reach == 0 && p.draft().length == 32 && p.draft().offset == -32, "new: 2 bars before the join, no reach");
        // Shift + encoder 2 up: 64 steps, the end on the join (offset -length), from the song's start.
        // Build 050: no bar length the song can't hold (the song starts 64 steps before the join): 4 bars is the last.
        p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 64 && p.draft().offset == -64 && p.draft().reach == 0 && s[0].length == 64, "4 bars, before the join: no reach");
        p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 64, "no scene ends for a window ending on the join (build 048), nor longer bars");
        // Over the join a bar: then the scene ends from its start (-48: 112, ...), the start kept.
        p.input(s, turn(0, 1), &names);
        require(p.draft().offset == -48, "a bar on, over the join");
        p.input(s, turn(1, 1, true), &names);
        // Build 058 (was: 112, until [1], 2 at once): the length that starts the window on the section before's start
        // (80 = 64 + its end, 16, kept) is now in the list ahead of the scene ends; the window's end stays.
        require(p.draft().length == 80 && p.draft().offset == -64 && p.draft().offset + 80 == 16, "build 058: on to the scene start (from [1], 1): the end kept, the start moved back");
        // The scene ends from there (-64: 128, 192, 224 and the song's end 256), the start kept.
        p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 128 && p.draft().offset == -64, "until [1], 2 from there");
        for (u32 i = 0; i < 6; ++i) p.input(s, turn(1, 1, true), &names);
        for (u32 i = 0; i < 40; ++i) p.input(s, turn(0, 1), &names);
        require(p.draft().offset == -64 && p.draft().length == 256, "until end: its end the end of the song's last entry; it can't slide on");
        const Draft& d = p.draft();
        require(d.reach == 3 && d.reach_start[0] == 64 && d.reach_start[1] == 128 && d.reach_start[2] == 160 &&
                    d.reach_scene[0] == 2 && d.reach_scene[1] == 3 && d.reach_scene[2] == 4,
                "reach: the three later joins, their scenes and distances");
        require(s[0].reach == 3 && s[0].length == 256 && s[0].offset == -64 && s[0].entry == 1, "stored with its reach, pinned to its join");
        u32 sc, st;
        s.reach(s[0], 3, &sc, &st);
        require(sc == 4 && st == 160, "the stored record");
        draw();
        require(std::strstr(f.texts[screen::t_join].chars, " to 5") != nullptr, "the title's join text names the last scene reached");
        // Build 049: the timeline holds the song the window may occupy; build 066 ("the left side of the screen
        // isn't anchored to the start"): that is now the song itself, -64..192, where it used to run to -256 - 192
        // steps of nothing before the song began, which the window could be moved into and which never played.
        // Marks at the section boundaries in view: the song's start (-64, on the view's left edge now, drawn because
        // build 066 made the test `>=`) and the three later joins (64, 128, 160); the variation's own join at 0.
        {
            const i32 from = -64, to = 192;
            require(near_x(f.nodes[screen::n_join].x, tl_x(0, from, to)), "the variation's join at step 0");
            i32 xs[screen::mark_count], count = 0;
            for (u32 i = 0; i < screen::mark_count; ++i)
                if (visible(f.nodes[screen::n_marks + i])) {
                    require(f.nodes[screen::n_marks + i].shade == screen::s_marker && f.nodes[screen::n_marks + i].y == screen::tl_cy * unit, "a scene marker (build 050)");
                    xs[count++] = f.nodes[screen::n_marks + i].x;
                }
            // Build 051: and the song's end (192, the view's right edge), named "end" left of it.
            const i32 steps[5] = {-64, 64, 128, 160, 192};
            require(count == 5, "five marks: the section before's start, three later joins and the song's end");
            for (i32 k = 0; k < 5; ++k) require(near_x(xs[k], tl_x(steps[k], from, to)), "each at its step");
            bool end_named = false;
            for (u32 t : {screen::t_label_a, screen::t_label_b})
                if (!std::strcmp(f.texts[t].chars, "end")) end_named = f.texts[t].x < tl_x(192, from, to) - 4 * unit;
            require(end_named, "\"end\" by the song's end, to its left");
            require(shown(screen::n_bars, screen::bar_count) == 0, "the ruler's marks are the shape page's");
        }
        // Build 048: the lengths now include the scene ends from where it starts (-64): to scene 3's end
        // (192 steps), to scene 4's (224), the song's end (256, a fixed length too). Two down from 256:
        // 192, the start kept; it crosses only the first join.
        p.input(s, turn(1, -2, true), &names);
        require(p.draft().length == 192 && p.draft().offset == -64 && p.draft().reach == 1 && p.draft().reach_start[0] == 64 &&
                    s[0].reach == 1 && s[0].length == 192,
                "shorter, to a scene's end: the start kept, the reach follows the window");
        // Build 049: named by the song page's grid (entry 3, index 2, ends at 128 from the join): row [1], column 3.
        p.input(s, shift_down, &names);
        draw();
        require(!std::strcmp(f.texts[screen::t_name_2].chars, "until") && !std::strcmp(f.texts[screen::t_value_2].chars, "[1], 3"),
                "the length: until / [1], 3 (build 052: name and value)");
        PageInput up_ = shift_down;
        up_.kind = PageInput::release, up_.shift = false;
        p.input(s, up_, &names);
        p.input(s, turn(1, -1, true), &names);
        require(p.draft().length == 128 && p.draft().offset == -64, "build 050: 128, a scene end (until [1], 2), the start kept");
        p.input(s, turn(1, -1, true), &names);
        require(p.draft().length == 64 && p.draft().offset == 0, "then the bar lengths, shorter than the first scene end: 64, the end kept");
        // Build 050: up to a scene end (from 0: 128, until [1], 3), back 4 bars to -64, up to the song's end.
        p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 128 && p.draft().offset == 0, "from 0, the next is a scene end");
        for (u32 i = 0; i < 4; ++i) p.input(s, turn(0, -1), &names);
        for (u32 i = 0; i < 9 && p.draft().length < 256; ++i) p.input(s, turn(1, 1, true), &names);
        for (u32 i = 0; i < 40; ++i) p.input(s, turn(0, 1), &names);
        require(p.draft().length == 256 && p.draft().offset == -64 && p.draft().reach == 3, "back to 256, slid to the far end: three joins again");
        // The playhead runs on across the later scenes.
        SongPosition pos = {true, 0, 2, 3, 10 * 480, 64 * 480, 2, kNoScene};  // entry 3 (index 2), step 10: 64 + 10 steps from the join
        p.follow(pos);
        draw();
        require(p.playhead() >= 0, "the playhead is shown in a later entry");
        const i32 here = p.playhead();
        pos.entry = 3, pos.position = 4 * 480, pos.scene = 3;  // entry index 3: 128 + 4
        p.follow(pos);
        draw();
        require(p.playhead() > here, "and runs on across the next join");
        pos.entry = 4, pos.position = 0;  // the last entry the window reaches (160 steps from the join)
        p.follow(pos);
        draw();
        require(p.playhead() > here, "into the last entry");
        // A shorter song: no later entries, no reach.
        static SongShape lone = {2, {0, 1}, {64, 64}};
        Store t;
        VariationPage q;
        Around b = a;
        b.entries = 2, b.shape = &lone;
        q.open(t, b, heard, &names);
        for (u32 i = 0; i < 3; ++i) q.input(t, turn(1, 1, true), &names);
        require(q.draft().length == 64 && q.draft().offset == -64, "build 050: no longer than the song holds (64 steps before the join)");
        for (u32 i = 0; i < 40; ++i) q.input(t, turn(0, 1), &names);
        require(q.draft().length == 64 && q.draft().offset == 0 && q.draft().reach == 0, "the song's last entry: the window may not run past it");
    }
    // Build 049: windows to 64 bars, a reach past 15 joins, the shape page on a stretched line.
    // Song: 40 entries of 64 steps; the page on the join into entry 2 (index 1).
    {
        static SongShape shape = {};
        shape.entries = 40;
        for (u32 i = 0; i < 40; ++i) shape.scene[i] = static_cast<u8>(i % 8), shape.steps[i] = 64;
        Store s;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 40, a.entry = 1, a.scene_before = 0, a.scene_after = 1;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        p.open(s, a, heard, &names);
        PageFrame f = {};  // zeroed: the page writes no header, and draw::valid checks those strings
        auto draw = [&]() { p.draw(PageTick{16, true}, f); };
        // Build 050: 4 bars (all the song holds before the join), moved to the join, then the scene ends
        // from there (64, 128, ... steps) up to 1024 (64 bars), no further.
        p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 64 && p.draft().offset == -64, "4 bars, all the song holds before the join");
        for (u32 i = 0; i < 4; ++i) p.input(s, turn(0, 1), &names);
        require(p.draft().offset == 0, "moved onto the join");
        for (u32 i = 0; i < 40 && p.draft().length < 1024; ++i) p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 1024 && p.draft().offset == 0 && s[0].length == 1024, "1024 steps, the start kept");
        // Build 063 (version 12): 1024 is not the limit now; the next scene end is (it said "no longer than 64 bars").
        p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 1088 && s[0].length == 1088, "on past 64 bars to the next scene end");
        p.input(s, turn(1, -1, true), &names);
        require(p.draft().length == 1024, "and back to 1024");
        p.input(s, turn(1, -1, true), &names);
        require(p.draft().length == 960, "one down from it");
        p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 1024, "up again");
        draw();
        // The length text (shown under Shift) says bars.
        PageInput down = press(keys::shift);
        down.shift = true;
        p.input(s, down, &names);
        draw();
        require(!std::strcmp(f.texts[screen::t_name_2].chars, "until") && !std::strcmp(f.texts[screen::t_value_2].chars, "[17], 1"),
                "the length: until / [17], 1 (entry 17)");
        PageInput released = down;
        released.kind = PageInput::release, released.shift = false;
        p.input(s, released, &names);
        // A reach past 15: slide the 1024-step window over the join and on. Build 070: the start is no longer
        // held inside the join's own section, so it runs on until the window's end meets the song's end. Assert that
        // invariant - it is at its bound and stays there - rather than the old "offset < 64", which was start_cap
        // written into an expectation.
        for (u32 i = 0; i < 130; ++i) p.input(s, turn(0, 1), &names);
        const i32 far_offset = p.draft().offset;
        require(far_offset >= 0 && p.draft().length == 1024, "slid to the start side");
        p.input(s, turn(0, 1), &names);
        require(p.draft().offset == far_offset, "070: and it is at its bound, the song's end holding it");
        require(p.draft().reach > 15 && p.draft().reach <= kMaxReach && s[0].reach == p.draft().reach, "a reach of more than 15 joins");
        u32 sc = 0, st = 0;
        s.reach(s[0], p.draft().reach, &sc, &st);
        require(st == 64 * p.draft().reach && sc == (p.draft().reach + 1) % 8, "the last record");
        // The shape page: the cursor in the line's 256 steps, whatever the window's length.
        p.input(s, press(keys::m1 + 2), &names);
        require(p.on_points() && p.cursor() == 0, "the shape page");
        p.input(s, turn(0, 1000), &names);
        require(p.cursor() == 255, "the cursor stops at the line's last step (255), not the window's (1023)");
        p.input(s, press(keys::plus), &names);
        require(p.cursor() == 255, "plus: no further");
        // Build 071 ("two very close points, one is skipped"): - and + go to the previous / next point, not 16
        // steps (was "minus: 16 line steps back" to 239); the line's points are wherever they are, so it is the last
        // point behind 255 (or stays if none), and the encoder goes to 239 as the rest of this block needs.
        {
            u32 pn = 0;
            const Point* pp = s.points(s[0], 0, &pn);
            u32 want = 255;
            for (u32 k = 0; k < pn; ++k) if (pp[k].step < 255) want = pp[k].step;
            p.input(s, press(keys::minus), &names);
            require(p.cursor() == want, "071: minus: the previous point");
            p.input(s, turn(0, 1000), &names);
            require(p.cursor() == 255, "back to the end");
        }
        p.input(s, turn(0, -16), &names);
        require(p.cursor() == 239, "16 line steps back");
        p.input(s, press(keys::step + 15, true), &names);  // Shift + a step key: no point there, nothing to remove
        p.input(s, turn(1, 5), &names);                    // a point on the cursor's step
        draw();
        u32 n = 0;
        const Point* pts = s.points(s[0], 0, &n);
        bool at = false;
        for (u32 k = 0; k < n; ++k) at = at || pts[k].step == 239;
        require(at && pts[n - 1].step < 256, "a point at line step 239; the line stays within 256 steps");
        draw();
        PageLeds leds = {};
        p.leds(leds);
        u32 pk = 0;
        while (pk < n && pts[pk].step != 239) ++pk;
        require(n <= 16 && pk < n && leds.step[pk] == led::on && (pk == 0 || leds.step[0] == led::dim) && (n >= 16 || leds.step[n] == led::off),
                "071: the cursor's point's key on, the line's other points dim, past the last off (was: step key 16 of page 15)");
        p.input(s, press(keys::m1 + 2), &names);
    }
    // Build 049: "until end" on a long song: 12 entries of 64 steps, the window starting inside entry 2.
    {
        static SongShape shape = {};
        shape.entries = 12;
        for (u32 i = 0; i < 12; ++i) shape.scene[i] = static_cast<u8>(i), shape.steps[i] = 64;
        Store s;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 12, a.entry = 1, a.scene_before = 0, a.scene_after = 1;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        p.open(s, a, heard, &names);
        PageFrame f = {};  // zeroed: the page writes no header, and draw::valid checks those strings
        for (u32 i = 0; i < 9 && p.draft().length < 64; ++i) p.input(s, turn(1, 1, true), &names);  // build 050: 64, the song's start
        for (u32 i = 0; i < 12; ++i) p.input(s, turn(0, 1), &names);  // over the join, to the start side
        require(p.draft().length == 64 && p.draft().offset > -64 && p.draft().offset + 64 > 0, "over the join");
        PageInput down = press(keys::shift);
        down.shift = true;
        p.input(s, down, &names);
        bool found = false;
        for (u32 i = 0; i < 12 && !found; ++i) {
            p.input(s, turn(1, 1, true), &names);
            p.draw(PageTick{16, true}, f);
            found = !std::strcmp(f.texts[screen::t_name_2].chars, "until") && !std::strcmp(f.texts[screen::t_value_2].chars, "end");
        }
        require(found && p.draft().length > 256 && p.draft().offset + static_cast<i32>(p.draft().length) == 11 * 64, "the length to the song's end, past 256 steps");
    }
    // Build 065 (from a device photo reading "move +63s"): moving right used to clamp the window's start onto
    // highest()'s raw bound. That bound is steps_after - 1, one step inside the window's own section, so the start
    // came to rest off every grid (63 steps is three bars and fifteen steps) and the band's edge sat a step away
    // from the scene marker, which reads as the marker being swallowed. The clamp now lands on the last position
    // the snap allows; a second press still reaches the bound, so a window flush to a boundary is not lost.
    {
        static SongShape shape = {4, {0, 1, 2, 3}, {64, 64, 64, 64}};
        Store st;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 4, a.entry = 1, a.scene_before = 0, a.scene_after = 1;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        p.open(st, a, heard, &names);
        // The page opens on the bar snap (snap_ = 2, kSnaps[2] = 16).
        require(p.draft().offset == -32 && p.draft().length == 32, "065: new, 2 bars before the join");
        for (u32 i = 0; i < 5; ++i) p.input(st, turn(0, 1), &names);  // -16, 0, 16, 32, 48
        require(p.draft().offset == 48 && p.draft().offset % 16 == 0,
                "065: moving right stops on the snap's grid, not on the raw bound (was 63 at once)");
        // Build 070 ("I can't move it to the end so I have to switch from bars to beats to get it to the end,
        // and then find it doesn't perfectly fit, when the song is an even numbers of bars"). Build 047 held the
        // start inside its own section, one step short of the next join, so on this song - four 64-step entries,
        // the page on the join into entry 1, so 192 steps of song ahead - a 32-step window could never end on the
        // song's end. It reaches it now: the bound is 160, the window ends at 192, and it is on the bar grid, so
        // holding the encoder lands there exactly.
        for (u32 i = 0; i < 20; ++i) p.input(st, turn(0, 1), &names);
        require(p.draft().offset == 160, "070: the start runs on to where the window ends on the song's end");
        require(p.draft().offset + static_cast<i32>(p.draft().length) == 192, "070: flush on the song's end");
        require(p.draft().offset % 16 == 0, "070: and on a whole bar");
    }
    // Build 066 ("I'm still able to move it further back than that... what is that space?", and "start is still
    // half way through the screen"). A window longer than the distance back to the song's start used to be allowed
    // back by its own length - build 024's rule, which never asked whether there was song there - so its leading
    // steps fell before the song began and never played. The start stops at the song's start now, and the view
    // follows, because extent() takes its lower bound from lowest().
    {
        // The song's first entry is one bar, the rest four, and the page is on the join into entry 1. The default
        // window is 32 steps, longer than the 16 steps of song behind that join, so build 024's length rule would
        // have let its start go to -32: sixteen steps before the song began.
        static SongShape shape = {6, {0, 1, 2, 3, 4, 5}, {16, 64, 64, 64, 64, 64}};
        Store st;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 6, a.entry = 1, a.scene_before = 0, a.scene_after = 1;
        a.steps_before = 16, a.steps_after = 64, a.shape = &shape;
        p.open(st, a, heard, &names);
        require(p.draft().length == 32, "066: the default window, longer than the song behind the join");
        require(p.lowest_start() == -16, "066: the start stops at the song's start, not at the window's own length");
        for (u32 i = 0; i < 40; ++i) p.input(st, turn(0, -1), &names);
        require(p.draft().offset == -16, "066: moving back lands on the song's start and goes no further");
        PageFrame fr = {};
        p.draw(PageTick{16, true}, fr);
        require(p.view_from() >= -16, "066: so the view cannot scroll before the song either");
    }
    // Entry 0 is untouched: the section before its join is the song's last scene arriving by the loop, and it plays.
    {
        static SongShape shape = {6, {0, 1, 2, 3, 4, 5}, {64, 64, 64, 64, 64, 64}};
        Store st;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 6, a.entry = 0, a.scene_before = 5, a.scene_after = 0;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        p.open(st, a, heard, &names);
        require(p.lowest_start() <= -64, "066: on entry 0 the start still reaches into the section before the join");
    }
    // Build 058 (2026-11-24: "I'd like to be able to have another variation span the entire song, despite it being
    // put at the first instance of scene 2"): the window's start back across several earlier scenes. Song: 6 entries of
    // 64 steps, scenes 1-6; the page on the join into entry 4 (index 3), three entries before it.
    {
        static SongShape shape = {6, {0, 1, 2, 3, 4, 5}, {64, 64, 64, 64, 64, 64}};
        Store s;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 6, a.entry = 3, a.scene_before = 2, a.scene_after = 3;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        p.open(s, a, heard, &names);
        PageFrame f = {};
        auto draw = [&]() { p.draw(PageTick{16, true}, f); };
        require(p.draft().length == 32 && p.draft().offset == -32, "058: new, 2 bars before the join");
        require(p.lowest_start() == -192, "058: lowest(): the start reaches back across all three earlier scenes, not one section");
        for (u32 i = 0; i < 40; ++i) p.input(s, turn(0, -1), &names);
        require(p.draft().offset == -192, "058: the start moved back across more than one earlier scene (to the song's start)");
        // Build 064: the start is exactly on entry 0's start (192 steps back), so that entry is recorded too (it was not at
        // 058: "from start" left a song-long variation silent for its whole first scene). Rewritten from back == 2.
        require(p.draft().back == 3 && p.draft().back_start[0] == 64 && p.draft().back_start[1] == 128 && p.draft().back_start[2] == 192 &&
                    p.draft().back_scene[0] == 2 && p.draft().back_scene[1] == 1 && p.draft().back_scene[2] == 0,
                "058/064: the backward list: the joins into entries 3, 2 and 1 (the song's start), their scenes and distances");
        // Stored, validated, and read back (the backward records stage A added).
        require(s[0].back == 3 && s[0].offset == -192 && s[0].length == 32 && s[0].entry == 3, "058: stored with its backward reach");
        u32 sc, st;
        s.back(s[0], 2, &sc, &st);
        require(sc == 1 && st == 128, "058: the stored backward record");
        s.back(s[0], 3, &sc, &st);
        require(sc == 0 && st == 192, "build 064: and the one for the song's first entry");
        require(Store::valid(p.draft()), "058: a window made that way validates");
        {
            VariationPage q;
            q.open(s, a, heard, &names);
            require(q.draft().offset == -192 && q.draft().length == 32 && q.draft().back == 3 && q.draft().back_start[1] == 128, "058: and opens again as it was");
        }
        // The length list: back at -32 (end 0), the lengths that start it on an earlier join keep its end.
        for (u32 i = 0; i < 10; ++i) p.input(s, turn(0, 1), &names);
        require(p.draft().offset == -32, "058: back to ending on the join");
        for (u32 i = 0; i < 40; ++i) p.input(s, turn(1, -1, true), &names);
        require(p.draft().length == 8 && p.draft().offset == -8, "058: shortest, the end kept");
        const i32 end = 0;
        PageInput shift_down = press(keys::shift);
        shift_down.shift = true;
        PageInput up_ = shift_down;
        up_.kind = PageInput::release, up_.shift = false;
        bool seen[3] = {};
        for (u32 i = 0; i < 20; ++i) {
            p.input(s, turn(1, 1, true), &names);
            require(p.draft().offset + static_cast<i32>(p.draft().length) == end, "058: lengths up the list keep the end (no scene ends before the join)");
            p.input(s, shift_down, &names);
            draw();
            const char* name = f.texts[screen::t_name_2].chars;
            const char* value = f.texts[screen::t_value_2].chars;
            texts_check::texts_ok(f, p.on_points());  // both halves in the strip's 55 px cell (estimate), and by the device's measure:
            {
                auto saved = draw::host_text_measure();
                draw::host_text_measure() = texts_check::device_like;
                texts_check::texts_ok(f, p.on_points());
                draw::host_text_measure() = saved;
            }
            p.input(s, up_, &names);
            if (!std::strcmp(name, "from")) {
                if (!std::strcmp(value, "[1], 3") && p.draft().length == static_cast<u32>(end) + 64) seen[0] = true;
                if (!std::strcmp(value, "[1], 2") && p.draft().length == static_cast<u32>(end) + 128) seen[1] = true;
                if (!std::strcmp(value, "start") && p.draft().length == static_cast<u32>(end) + 192) seen[2] = true;
            }
        }
        require(seen[0] && seen[1] && seen[2], "058: the column reads from / [1], 3, from / [1], 2 and from / start for the three earlier joins");
        require(p.draft().offset == -192 && p.draft().length == 192 && p.draft().back == 3, "058 (064: back 3, the first entry recorded): it ends at the song's start, the window from there to the join");
        require(Store::valid(p.draft()) && s[0].back >= 1, "058: the window made by a length validates and is stored with back records");
    }
    // Build 058: without the song's shape the list is as it was (the fixed lengths, the end kept), no scene starts.
    {
        Store s;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 6, a.entry = 3, a.scene_before = 2, a.scene_after = 3;
        a.steps_before = 64, a.steps_after = 64;
        p.open(s, a, heard, &names);
        require(p.lowest_start() == -64, "058: no shape: the start reaches back one section as before");
        static const u32 want[] = {64, 128, 256, 512, 1024};
        u32 at = 0;
        for (u32 i = 0; i < 8; ++i) {
            p.input(s, turn(1, 1, true), &names);
            if (p.draft().length == want[at]) ++at;
            require(p.draft().offset + static_cast<i32>(p.draft().length) == 0, "058: no shape: every length keeps the end");
        }
        require(at == 5, "058: no shape: the fixed lengths in order, none added");
    }
    // Build 058: the longest window asked for: pinned partway into a song, grown to span from the song's start to its
    // end. 16 entries of 64 steps (1024: kMaxLength until build 063), the page on the join into entry 9 (index 8).
    {
        static SongShape shape = {};
        shape.entries = 16;
        for (u32 i = 0; i < 16; ++i) shape.scene[i] = static_cast<u8>(i % 8), shape.steps[i] = 64;
        Store s;
        VariationPage p;
        Around a = {};
        a.song = true, a.song_index = 0, a.entries = 16, a.entry = 8, a.scene_before = 7, a.scene_after = 0;
        a.steps_before = 64, a.steps_after = 64, a.shape = &shape;
        p.open(s, a, heard, &names);
        PageFrame f = {};
        require(p.lowest_start() == -512, "058: the start may reach the song's start, 8 scenes back");
        // Grown by the bar lengths (the end kept, on the join: 512 is the most the song holds before it), then a bar over
        // the join so the scene ends are offered, and up the list: the scene start that puts the start on the song's start
        // (528), then the scene ends from there, the last "until end" (1024).
        for (u32 i = 0; i < 40 && p.draft().length < 512; ++i) p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 512 && p.draft().offset == -512, "058: 512, the song's first half");
        p.input(s, turn(0, 1), &names);
        for (u32 i = 0; i < 40 && p.draft().length < 1024; ++i) p.input(s, turn(1, 1, true), &names);
        require(p.draft().length == 1024 && p.draft().offset == -512, "058: 1024 steps, from the song's start");
        p.input(s, shift_down, &names);
        p.draw(PageTick{16, true}, f);
        require(!std::strcmp(f.texts[screen::t_name_2].chars, "until") && !std::strcmp(f.texts[screen::t_value_2].chars, "end"), "058: it ends on the song's end");
        require(p.draft().reach == 7 && p.draft().back == 8 && s[0].back == 8 && s[0].reach == 7 && Store::valid(p.draft()), "058: seven joins forward, and (064) eight back, the song's first entry included, stored");
        // Build 063 (version 12: "I'd like a song spanning variation to be an option"): one more entry than 1024 steps
        // used to be more than a window could span (kMaxLength was 1024); it spans it now.
        static SongShape longer = {};
        longer.entries = 17;
        for (u32 i = 0; i < 17; ++i) longer.scene[i] = static_cast<u8>(i % 8), longer.steps[i] = 64;
        Store t;
        VariationPage q;
        Around b = a;
        b.entries = 17, b.shape = &longer;
        q.open(t, b, heard, &names);
        for (u32 i = 0; i < 40 && q.draft().length < 512; ++i) q.input(t, turn(1, 1, true), &names);
        q.input(t, turn(0, 1), &names);
        for (u32 i = 0; i < 60; ++i) q.input(t, turn(1, 1, true), &names);
        require(q.draft().length == 17 * 64 && q.draft().offset == -512 && q.draft().reach == 8 && q.draft().back == 8 && Store::valid(q.draft()),
                "063: a 1088 step song is spanned now, from its start to its end");
        // A song of 65 entries of 64 steps is 4160: one more bar than kMaxLength, so a window still cannot span it.
        static SongShape huge = {};
        huge.entries = 65;
        for (u32 i = 0; i < 65; ++i) huge.scene[i] = static_cast<u8>(i % 8), huge.steps[i] = 64;
        Store u;
        VariationPage w;
        Around c = a;
        c.entries = 65, c.entry = 32, c.shape = &huge, c.scene_before = 7, c.scene_after = 0;
        w.open(u, c, heard, &names);
        for (u32 i = 0; i < 80 && w.draft().length < 2048; ++i) w.input(u, turn(1, 1, true), &names);
        w.input(u, turn(0, 1), &names);
        for (u32 i = 0; i < 300; ++i) w.input(u, turn(1, 1, true), &names);
        for (u32 i = 0; i < 300; ++i) w.input(u, turn(0, -1), &names);
        require(w.draft().length <= kMaxLength && w.draft().length < 65 * 64 && w.draft().length > 2048,
                "063: kMaxLength is still the limit: at most 4096 steps, so a 4160 step song cannot be spanned");
    }
    std::puts("variations page: ok");
    return 0;
}
