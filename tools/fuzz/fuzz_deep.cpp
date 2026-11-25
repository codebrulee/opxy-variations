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
static Around song_1() {
    Around a = {};
    a.song = true, a.song_index = 0, a.entries = 12, a.entry = 6;
    a.scene_before = 0, a.scene_after = 1;
    a.steps_before = 64, a.steps_after = 64; static SongShape sh = {12, {0,1,2,3,4,5,6,7,0,1,2,3},
        {64,64,32,32,64,64,64,32,32,64,64,64}}; a.shape = &sh;
    return a;
}
static u32 seed = 1;
static u32 rnd(u32 n) { seed = seed * 1664525u + 1013904223u; return (seed >> 16) % n; }
int main(int argc, char** argv) {
    const u32 runs = argc > 1 ? static_cast<u32>(std::atoi(argv[1])) : 4000;
    if (argc > 2) seed = static_cast<u32>(std::atoi(argv[2]));
    Draft d = {};
    d.song = 0, d.entry = 6, d.offset = 0, d.length = 32, d.scene_before = kNoScene, d.scene_after = kNoScene;
    DraftLane& l = d.lanes[0];
    l.gesture = rise, l.selection = 0x14, l.amount = 128, l.count = 2, l.default_target = true;
    l.points[0] = Point{0, 0, 0};
    l.points[1] = Point{31, 255, 0};
    store.put(0, d);
    page.open(store, song_1(), heard);
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
        if (page.input(store, in)) { page.open(store, song_1(), heard); continue; }
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
    std::printf("ok: %u runs clean (seed end %u)\n", runs, seed);
    return 0;
}
