// Build 026: the page's centred and right-aligned texts placed by the device's
// own text measure (opxy::draw::text_width), here fake metrics installed as a
// host test does: every centred text (title, the shape page's target, the message) on its centre as stock centres (x = c - w / 2
// in whole pixels), every right-aligned one (the tracks) ending exactly at the right edge, and (build 052) the control
// strip's names and values left aligned at stock's anchors within 55 px, on both pages and for
// several gestures; without the measure, the estimate. Build 028: below 15 px
// (hinted) the width is the measure plus 3/4 px a glyph after the first, as
// the device drew it (notes/variations-page-026.md); 30 px the measure alone.
#include "../page.hpp"
#include "texts.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace variations;
using opxy::extensions::PageInput;
using opxy::extensions::PageTick;
namespace draw = opxy::draw;
namespace keys = opxy::extensions::keys;

static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
using screen::unit;

// Fake metrics, unlike the estimate: a glyph is (size + its code's low bits) / 2
// pixels, odd and even sums alike, so whole-pixel rounding shows. Counts calls.
static u32 calls;
static i32 fake(i32 size, const char* text) {
    ++calls;
    i32 w = 0;
    for (const char* c = text; *c; ++c) w += (size + (*c & 7)) / 2;
    return w;
}
static i32 px(const char* s, i32 size) { return fake(size, s); }
// The width the page places by, Q10: the measure, plus the hinted track below 15 px.
static i32 wq(const char* s, i32 size) {
    const i32 n = static_cast<i32>(std::strlen(s));
    return px(s, size) * unit + (size < 15 && n > 1 ? (n - 1) * 3 * unit / 4 : 0);
}
static i32 half(i32 w) { return w % unit ? w / 2 : (w / unit / 2) * unit; }

static Store store;
static Heard heard;
static VariationPage page;
static PageFrame frame;

static bool no_names(u32, u32, char* text, u32 capacity) {
    if (capacity) text[0] = 0;
    return false;
}
static const Names names = {no_names};
static PageInput turn(u32 encoder, i32 step) {
    PageInput in = {};
    in.kind = PageInput::turn, in.encoder = encoder, in.step = step;
    return in;
}
static PageInput press(u32 key) {
    PageInput in = {};
    in.kind = PageInput::press, in.key = key;
    return in;
}
static void give(const PageInput& in) { require(!page.input(store, in, &names), "the page never closes itself"); }

static void centred_on(u32 slot, i32 c, i32 size) {
    const draw::Text& t = frame.texts[slot];
    if (!t.chars[0]) return;
    const i32 want = c * unit - half(wq(t.chars, size));
    if (t.x != want) {
        std::fprintf(stderr, "slot %u \"%s\": x %d, centred on %d wants %d (1/1024 px)\n", slot, t.chars, t.x, c, want);
        require(false, "centred by the measure");
    }
}
static void frame_ok() {
    using namespace screen;
    const PageTick tick = {16, true};
    calls = 0;
    page.draw(tick, frame);
    require(draw::valid(frame) && calls > 0, "a valid frame, measured");
    texts_check::texts_ok(frame, page.on_points());
    centred_on(t_title, 240, header_size);
    // Build 052: t_number_a is the shape page's centred target (10 px); on the variation page it holds the window's
    // bar range, at its line (tests/page.cpp).
    if (page.on_points()) centred_on(t_number_a, 240, target_size);
    // Build 056: no track list in the header; t_mark_m2 is a centred mark (texts_check::texts_ok checks it with the others).
    // Build 052: the strip is left aligned at stock's anchors, not justified (texts_check::texts_ok): the names and
    // values start where they are authored, whatever their measure.
    for (u32 c = 0; c < 4; ++c) {
        const draw::Text& name = frame.texts[t_name_1 + c];
        const draw::Text& value = frame.texts[t_value_1 + c];
        require(!name.chars[0] || name.x == (name_x + static_cast<i32>(c) * column_pitch) * unit, "a strip name left aligned");
        require(!value.chars[0] || value.x == (value_x + static_cast<i32>(c) * column_pitch) * unit, "a strip value left aligned");
        require(words::width(name.chars, names_size) <= strip_room * unit && words::width(value.chars, value_size) <= strip_room * unit,
                "a strip name and value within 55 px by the measure");
    }
    if (page.on_points()) return;
    // Build 054: t_number_a is centred while a message shows; otherwise it holds the window's bar range, at its line.
    if (page.message()) centred_on(t_number_a, 240, message_size);
    // Build 049: the scene names and bar numbers sit on their lines, not on the sides (tests/page.cpp).
}

int main() {
    using namespace screen;
    // Without a measure: the estimate.
    require(draw::text_width(12, "shape") == -1 && words::width("shape", 12) == words::estimate("shape", 12), "no measure");
    draw::host_text_measure() = fake;
    require(words::width("shape", 30) == px("shape", 30) * unit && words::width("", 30) == 0 && words::width("", 12) == 0,
            "30 px: the measure alone, Q10");
    require(words::width("shape", 12) == px("shape", 12) * unit + 3 * unit, "12 px: plus 3/4 px for each of 4 glyphs after the first");
    require(words::width("variation 1 of 5", 10) == px("variation 1 of 5", 10) * unit + 45 * unit / 4 && words::width("x", 12) == px("x", 12) * unit,
            "10 px likewise; one glyph: the measure");
    require(words::centred_x(240 * unit, "shape", 12) == 240 * unit - half(wq("shape", 12)), "centred on the drawn width");
    require(words::centred_x(240 * unit, "rise", 30) == 240 * unit - (px("rise", 30) / 2) * unit, "30 px: centred as stock");
    require(words::ending_x(464 * unit, "1 2", 12) == 464 * unit - wq("1 2", 12), "ending at the edge");
    {
        // fit by the measure: whole words that fit, else cut.
        char out[draw::text_capacity];
        words::fit(out, "filter release env", wq("filter release", 12), 12);
        require(!std::strcmp(out, "filter release"), "fitted by the measure");
    }

    // A song of two entries, a new variation at the join into entry 2.
    Around a = {};
    a.song = true, a.song_index = 0, a.entries = 2, a.entry = 1;
    a.scene_before = 0, a.scene_after = 1, a.steps_before = 64, a.steps_after = 64;
    page.open(store, a, heard, &names);
    require(page.showing(), "opens");
    give(press(keys::track + 0));
    give(press(keys::track + 2));
    frame_ok();
    // Several gestures: each centred on 240 by its own width.
    char seen[8][draw::text_capacity] = {};
    u32 kinds = 0;
    for (u32 g = 0; g < 8; ++g) {
        give(turn(1, 1));
        frame_ok();
        bool fresh = true;
        for (u32 k = 0; k < kinds; ++k) fresh = fresh && std::strcmp(seen[k], frame.texts[t_value_2].chars) != 0;
        if (fresh && kinds < 8) std::strcpy(seen[kinds++], frame.texts[t_value_2].chars);
    }
    require(kinds >= 3, "several gestures seen");
    // The shape page (encoder 2's tap): the title, target and tracks as measured.
    give(press(keys::m1 + 2));
    require(page.on_points(), "the shape page");
    frame_ok();
    require(!std::strcmp(frame.texts[t_title].chars, "edit shape"), "its title");
    give(press(keys::m1 + 2));
    require(!page.on_points(), "and back");
    frame_ok();
    draw::host_text_measure() = nullptr;
    std::puts("variations metrics: ok");
    return 0;
}
