// Fuzz: random page inputs, every frame must stay valid (build 052 freeze report).
#include "../../page.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace variations;
using opxy::extensions::PageInput;
namespace draw = opxy::draw;
using screen::unit;
static Store store;
static Heard heard;
static VariationPage page;
static PageFrame frame;
static SongShape long_shape;
static Around song_1() {
    // Build 063: a 96-entry song (stock's most) of 16..96-step scenes, the page on the join into entry 48.
    Around a = {};
    long_shape.entries = 96;
    u32 x = 12345;
    for (u32 i = 0; i < 96; ++i) {
        x = x * 1664525u + 1013904223u;
        long_shape.scene[i] = static_cast<u8>((x >> 16) % 12);
        long_shape.steps[i] = 16 + ((x >> 8) & 3) * 16 + ((x >> 20) & 1) * 16;
    }
    a.song = true, a.song_index = 0, a.entries = 96, a.entry = 48;
    a.scene_before = long_shape.scene[47], a.scene_after = long_shape.scene[48];
    a.steps_before = long_shape.steps[47], a.steps_after = long_shape.steps[48];
    a.shape = &long_shape;
    return a;
}
static u32 seed = 1;
static u32 rnd(u32 n) { seed = seed * 1664525u + 1013904223u; return (seed >> 16) % n; }
int main(int argc, char** argv) {
    const u32 runs = argc > 1 ? static_cast<u32>(std::atoi(argv[1])) : 4000;
    const bool grow = argc > 3;
    if (argc > 2) seed = static_cast<u32>(std::atoi(argv[2]));
    Draft d = {};
    d.song = 0, d.entry = 48, d.offset = 0, d.length = 32, d.scene_before = kNoScene, d.scene_after = kNoScene;
    DraftLane& l = d.lanes[0];
    l.gesture = rise, l.selection = 0x14, l.amount = 128, l.count = 2, l.default_target = true;
    l.points[0] = Point{0, 0, 0};
    l.points[1] = Point{31, 255, 0};
    store.put(0, d);
    page.open(store, song_1(), heard);
    u32 longest = 0, most_reach = 0, most_back = 0;
    for (u32 r = 0; r < runs; ++r) {
        PageInput in = {};
        const u32 kind = rnd(10);
        const bool shift = rnd(3) == 0;
        if (kind < 5) {
            in.kind = PageInput::turn, in.encoder = rnd(4), in.step = rnd(2) ? 1 : -1, in.shift = shift;
            if (rnd(4) == 0) in.step *= 4;
        } else if (kind < 7) {
            in.kind = PageInput::click, in.encoder = rnd(4), in.key = keys::encoder + in.encoder, in.shift = shift;
        } else {
            in.kind = PageInput::press, in.shift = shift;
            const u32 k = rnd(5);
            in.key = k == 0 ? keys::step + rnd(16) : k == 1 ? keys::track + rnd(8) : k == 2 ? keys::m1 + rnd(4)
                   : k == 3 ? keys::minus : keys::plus;
        }
        // Biased towards growing: Shift + encoder 2 up and encoder 1 moving, in bursts, so the window reaches its longest (build 063).
        if (grow && rnd(5) < 3) {
            in = PageInput{};
            const u32 g = rnd(8);
            if (g < 4) in.kind = PageInput::turn, in.encoder = 1, in.step = (r / 4000) % 2 ? -1 : 1, in.shift = true;
            else in.kind = PageInput::turn, in.encoder = 0, in.step = rnd(2) ? 4 : -4, in.shift = false;
        }
        if (page.input(store, in)) { page.open(store, song_1(), heard); continue; }
        // Build 063 invariants: the draft is a valid store draft, the store never over capacity, and the stored reach lists are the song's.
        if (!Store::valid(page.draft()) || store.size() > kCapacity) { std::printf("INVALID DRAFT after run %u: length %u offset %d reach %u back %u\n", r, page.draft().length, page.draft().offset, page.draft().reach, page.draft().back); return 1; }
        if (store.count()) {
            u8 sc[kMaxReach]; u16 st[kMaxReach];
            const u32 reach = reach_of(long_shape.scene, long_shape.steps, 96, 48, page.draft().offset, page.draft().length, sc, st);
            if (reach != page.draft().reach || store[0].reach != reach) { std::printf("REACH MISMATCH run %u: %u vs %u (len %u off %d)\n", r, reach, page.draft().reach, page.draft().length, page.draft().offset); return 1; }
            u8 bsc[kMaxReach]; u16 bst[kMaxReach];
            const u32 back = back_reach_of(long_shape.scene, long_shape.steps, 96, 48, page.draft().offset, page.draft().length, bsc, bst);
            if (back != page.draft().back || store[0].back != back) { std::printf("BACK MISMATCH run %u: %u vs %u (len %u off %d)\n", r, back, page.draft().back, page.draft().length, page.draft().offset); return 1; }
            if (page.draft().length > kMaxLength || page.draft().offset < -(i32)kMaxOffset || page.draft().offset > (i32)kMaxOffset) { std::printf("RANGE run %u\n", r); return 1; }
            if (page.draft().length > longest) longest = page.draft().length;
            if (reach > most_reach) most_reach = reach;
            if (back > most_back) most_back = back;
        }
        PageTick tick = {static_cast<i32>(rnd(64)), rnd(20) == 0};
        page.draw(tick, frame);
        if (!draw::valid(frame)) {
            std::printf("INVALID after run %u: points %d, length %u, offset %d, lane %u\n", r, (int)page.on_points(),
                        page.draft().length, page.draft().offset, page.lane_index());
            for (u32 pp = 0; pp < screen::paths; ++pp)
                for (u32 i = 0; i < screen::points; ++i) {
                    const draw::Point& pt = frame.paths[pp].points[i];
                    if (pt.x < -64 * unit || pt.x > 64 * unit || pt.y < -64 * unit || pt.y > 64 * unit) {
                        std::printf("  path %u point %u: %d,%d\n", pp, i, pt.x / unit, pt.y / unit);
                        break;
                    }
                }
            for (u32 i = 0; i < screen::nodes; ++i)
                if (!draw::placement(frame.nodes[i].x, frame.nodes[i].y, frame.nodes[i].scale))
                    std::printf("  node %u: %d,%d scale %d\n", i, frame.nodes[i].x / unit, frame.nodes[i].y / unit, frame.nodes[i].scale);
            for (u32 i = 0; i < screen::texts; ++i)
                if (!draw::placement(frame.texts[i].x, frame.texts[i].y, frame.texts[i].scale))
                    std::printf("  text %u \"%s\": %d,%d\n", i, frame.texts[i].chars, frame.texts[i].x / unit, frame.texts[i].y / unit);
            return 1;
        }
    }
    std::printf("ok: %u runs clean (seed end %u); longest window %u, most reach %u, most back %u\n", runs, seed, longest, most_reach, most_back);
    return 0;
}
