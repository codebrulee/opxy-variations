// Host test: notes.hpp ranks and chooses as probe 089 did.
#include "../notes.hpp"
#include <cstdio>
#include <cstdlib>
using namespace variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static PatternNotes hats, claps, kit;
static Ranking ranking;
int main() {
    // 16 hats at one velocity: downbeat last, then the half bar, then beats, 8ths, 16ths.
    hats.steps = 16;
    hats.count = 16;
    for (u32 s = 0; s < 16; ++s) hats.notes[s] = Note{static_cast<opxy::i32>(s * 480), 120, 42, 100};
    require(ranking.refresh(hats) && ranking.groups() == 16, "16 groups");
    require(ranking.key_at(15) == 0 && ranking.key_at(14) == 8 * 128, "downbeat, then the half bar, last");
    for (u32 r = 0; r < 8; ++r) require(ranking.key_at(r) / 128 % 2 == 1, "16ths first");
    // Thinning: the lowest share of groups drops; same every time.
    u32 dropped = 0;
    for (u32 s = 0; s < 16; ++s) {
        const u32 r = static_cast<u32>(ranking.rank(hats.notes[s]));
        if (!choose(64 << 8, r, 16, 480, 120, 100).play) ++dropped;
    }
    require(dropped == 8, "value 64 (half way to none) drops half");
    for (u32 s = 0; s < 16; ++s) {
        const u32 r = static_cast<u32>(ranking.rank(hats.notes[s]));
        require(!choose(0, r, 16, 480, 120, 100).play, "value 0 drops all");
        require(choose(kMiddle, r, 16, 480, 120, 100).play && !choose(kMiddle, r, 16, 480, 120, 100).extra,
                "the middle: as written");
    }
    // Busier: four claps a bar; at full every one gets an extra halfway, quieter and shorter.
    claps.steps = 16;
    claps.count = 4;
    for (u32 b = 0; b < 4; ++b) claps.notes[b] = Note{static_cast<opxy::i32>(b * 4 * 480), 960, 39, 110};
    require(ranking.refresh(claps) && ranking.groups() == 4, "rebuilt for the new pattern");
    const u32 gap = ranking.gap(claps.notes[0]);
    require(gap == 4 * 480, "a beat to the next clap");
    const NoteChoice c = choose(255 << 8, 0, 4, gap, 960, 110);
    require(c.play && c.extra && c.delay == 960 && c.velocity == 77, "an extra on the 8th, at 70 %");
    require(c.own_length <= c.delay && c.length < c.delay, "neither overlaps the other");
    u32 extras = 0;
    for (u32 b = 0; b < 4; ++b)
        if (choose((128 + 64) << 8, static_cast<u32>(ranking.rank(claps.notes[b])), 4, gap, 960, 110).extra) ++extras;
    require(extras == 2, "half way to busy: half the claps");
    require(ranking.gap(claps.notes[3]) == 4 * 480, "wraps round the pattern");
    // Drums: a step's pads rank apart, and a pad's gap is to its own next hit.
    kit.steps = 16;
    kit.percussion = true;
    kit.count = 3;
    kit.notes[0] = Note{0, 120, 36, 100};
    kit.notes[1] = Note{0, 120, 42, 100};
    kit.notes[2] = Note{4 * 480, 120, 42, 100};
    require(ranking.refresh(kit) && ranking.groups() == 3, "pads apart");
    require(ranking.gap(kit.notes[0]) == 16 * 480, "the kick's next kick is a bar away");
    require(ranking.gap(kit.notes[1]) == 4 * 480, "the hat's next hat");
    std::puts("variations notes: ok");
    return 0;
}
