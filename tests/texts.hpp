// Host test helper (build 022; build 023 the shared centre line): the page's texts at stock's sizes, within their
// rooms, and apart from each other and from the lane letters, on both pages.
// Each text's box: words::width across (the device's measure, or the estimate without one), its ascenders (0.75 of
// its size) above the baseline and descenders (0.22) below.
#pragma once
#include "../page.hpp"
#include <cstdio>
#include <cstdlib>

namespace texts_check {
using namespace variations;
using namespace variations::screen;
namespace draw = opxy::draw;

inline void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
// Stock's text sizes (its artwork, private/stock-art), the only ones the page uses.
inline bool stock_size(i32 size) {
    const i32 sizes[] = {10, 12, 15, 17, 20, 30, 40};
    for (i32 s : sizes)
        if (s == size) return true;
    return false;
}
// Each slot's size, as tools/page_svg.py authors it (build 052: the strip's names 10 px, values 15, the M4 mark 20).
inline i32 size_of(u32 slot) {
    switch (slot) {
    case t_title: return header_size;
    case t_join: return join_size;
    case t_mark_m2: return mark_size;  // build 056: the third M-key mark, "mute" over M2 (was the header's track list)
    case t_number_a: return label_size;  // the window's bar range; on the shape page the target (target_size, also 10); the messages
    case t_mark_side: return mark_size;  // build 054: the second M-key mark, 20 px like t_mark
    case t_label_a: case t_label_b: return label_size;
    case t_mark: return mark_size;
    case t_name_1: case t_name_2: case t_name_3: case t_name_4: return names_size;
    default: return value_size;  // t_value_1..4
    }
}
// Build 055: a mark is centred on its key, so its room is twice the smaller distance from the centre to a screen edge
// (0 and 480), pixels: centred in it, it stays on screen. M1's centre (39.95 px) is the nearest an edge.
inline i32 mark_room(i32 centre) {
    const i32 c = centre / draw::unit;
    return 2 * (c < 480 - c ? c : 480 - c);
}
// Each slot's room, pixels.
inline i32 room_of(u32 slot, bool points) {
    switch (slot) {
    case t_title: return header_width;
    case t_join: return join_width;
    case t_mark_m2: return mark_room(m2_x);  // build 056: centred on M2
    case t_number_a: return points ? target_width : numbers_width;
    case t_mark_side: return mark_room(m3_x);  // build 055: centred on M3 ; room is mark_room's, below
    case t_label_a: case t_label_b: return label_width;
    case t_mark: return mark_room(m4_x);  // build 055: centred on M4
    default: return strip_room;  // build 052: every strip name and value, 55 px
    }
}
// Build 052: a measure like the device's, for the tests whose strip words must fit 55 px at 15 px. The estimate
// (words::estimate: the same table, widened 7 % and plus tracking, so that capitals in rooms measured in them fit) is
// too wide for them: it makes "param 1" 58.5 px where the device measures 54. This is the estimate's per-glyph table
// without the margin and the tracking, each advance rounded to whole pixels as stock's measure sums them. Not the
// device's metrics, only near them (the device: volume 50, param 1 54, step key 56, [25], 8 42; here 48, 54, 55, 42).
inline i32 device_like(i32 size, const char* text) {
    static const unsigned short lower[26] = {560, 600, 510, 600, 570, 320, 600, 600, 250, 250, 540, 250, 900,
                                             600, 600, 600, 600, 360, 480, 340, 600, 510, 790, 510, 510, 470};
    i32 w = 0;
    for (; *text; ++text) {
        const char c = *text;
        const i32 m = c == ' ' || c == ',' ? 280 : c >= '0' && c <= '9' ? 560 : c >= 'a' && c <= 'z' ? lower[c - 'a']
                    : c == 'I' ? 260 : c == 'M' || c == 'W' ? 820 : c == '%' ? 800 : c == '-' || c == '/' || c == '.' ? 340
                    : c == '>' ? 600 : c == '[' || c == ']' ? 340 : c == '+' ? 560 : 640;
        w += (m * size * 100 / 107 + 500) / 1000;  // per mille of the size, less the 7 % margin, to whole pixels
    }
    return w;
}
struct Box { i32 x0, y0, x1, y1; };
inline bool overlap(const Box& a, const Box& b) { return a.x0 < b.x1 && b.x0 < a.x1 && a.y0 < b.y1 && b.y0 < a.y1; }

template <class F> void texts_ok(const F& frame, bool points) {
    const i32 u = draw::unit;
    for (u32 i = 0; i < texts; ++i) require(stock_size(size_of(i)), "a stock text size");
    require(stock_size(header_size) && stock_size(join_size) && stock_size(small_size) && stock_size(label_size) &&
                stock_size(message_size) && stock_size(target_size) && stock_size(names_size) && stock_size(value_size) &&
                stock_size(mark_size),
            "every size stock's");
    Box boxes[texts + 1];
    u32 slots[texts + 1];
    u32 count = 0;
    for (u32 i = 0; i < texts; ++i) {
        const draw::Text& t = frame.texts[i];
        if (!t.chars[0]) continue;
        require(t.scale == u, "no scaled text");
        for (const char* c = t.chars; *c; ++c) require(*c < 'A' || *c > 'Z', "texts in stock's lowercase");
        const i32 size = size_of(i), w = words::width(t.chars, size);
        if (words::length(t.chars) > text_chars || w > room_of(i, points) * u) {
            std::fprintf(stderr, "slot %u \"%s\": %d px, room %d\n", i, t.chars, w / u, room_of(i, points));
            require(false, "texts within their room");
        }
        require(t.x >= 0 && t.x + w <= 480 * u, "texts on screen");
        slots[count] = i;
        boxes[count++] = {t.x, t.y - size * 3 * u / 4, t.x + w, t.y + size * 22 * u / 100};
        // Build 052: the strip's names and values left aligned at stock's anchors (x 5, 125, 245, 365 and 65, 185, 305,
        // 425; baselines 10 and 16.5), every one within 55 px.
        for (u32 c = 0; c < 4; ++c) {
            if (i == t_name_1 + c)
                require(t.x == (name_x + static_cast<i32>(c) * column_pitch) * u && t.y == strip_name_y * u, "a strip name at its anchor");
            if (i == t_value_1 + c)
                require(t.x == (value_x + static_cast<i32>(c) * column_pitch) * u && t.y == strip_value_y, "a strip value at its anchor");
        }
        // Build 055: the marks are centred on their keys, within a pixel (stock centres by whole pixels), on
        // the baseline, and each mark's box is on screen. Build 056: three marks - t_mark_m2 over M2, t_mark_side over M3,
        // t_mark over M4 ("mute" no longer sits over M1).
        if (i == t_mark || i == t_mark_side || i == t_mark_m2) {
            const i32 centre = t.x + w / 2, near = i == t_mark ? m4_x : i == t_mark_side ? m3_x : m2_x;
            const i32 d = centre - near < near - centre ? near - centre : centre - near;
            require(d <= u, "a mark centred on its key");
            require(t.y == m_y * u && t.x >= 0 && t.x + w <= 480 * u, "a mark on its baseline and on screen");
        }
        // Build 023: the title and the target share one centre line, x 240, by the same width (within a pixel).
        // Build 049: the target's slot names a third section on the variation page (and the gesture's holds the
        // bar numbers there); the centre line is the shape page's. (Build 052: t_number_a, shape page only.)
        if (i == t_title || (points && i == t_number_a)) {
            const i32 off = t.x + w / 2 - 240 * u;
            if (off > u || off < -u) {
                std::fprintf(stderr, "slot %u \"%s\": centre %d/1024 px off 240\n", i, t.chars, off);
                require(false, "the title, gesture and target on one centre line");
            }
        }
    }
    // The lane letters and encoder 1's dot, when shown: a glyph row at M1's anchor, 3/2 scale (build 052; 12 px
    // glyphs, 18 px here).
    const draw::Node& ring = frame.nodes[n_ring_a];
    if (ring.x != draw::hidden_x * u) {
        slots[count] = texts;
        boxes[count++] = {letters_x - u, letters_y * u - 15 * u, letters_x + letters_width * letters_scale, letters_y * u + u};
    }
    for (u32 a = 0; a < count; ++a)
        for (u32 b = a + 1; b < count; ++b) {
            // Build 054: the two M-key marks share the M row; their rooms run to the screen's edge, so the boxes
            // overlap there by room alone. Their anchors are 111 px apart and tests/page.cpp checks the words fit.
            // Build 056: the third mark, over M2, likewise.
            auto mark = [](u32 s) { return s == t_mark || s == t_mark_side || s == t_mark_m2; };
            if (mark(slots[a]) && mark(slots[b])) continue;
            if (overlap(boxes[a], boxes[b])) {
                std::fprintf(stderr, "boxes %u and %u overlap: (%d,%d)-(%d,%d) and (%d,%d)-(%d,%d)\n", a, b, boxes[a].x0 / u,
                             boxes[a].y0 / u, boxes[a].x1 / u, boxes[a].y1 / u, boxes[b].x0 / u, boxes[b].y0 / u,
                             boxes[b].x1 / u, boxes[b].y1 / u);
                require(false, "texts apart");
            }
        }
}
}  // namespace texts_check
