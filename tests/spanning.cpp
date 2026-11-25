// Host test: build 063's song-spanning variation (format version 12; "you mentioned something about a 64 bar
// limit, so I can't make a variation span an entire song if it's longer than that?", "I'd like a song spanning
// variation to be an option"). A 96-entry song of 32-step scenes (2 bars each: 3072 steps, 192 bars: more than
// kMaxLength was, 1024), the page on the join into entry 40, grown on the page until its window covers the whole
// song, then played back through the store in entries at both ends and in the middle.
#include "../page.hpp"
#include "../window.hpp"
#include <cstdio>
#include <cstdlib>
using namespace variations;
using opxy::extensions::PageInput;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static PageInput turn(u32 encoder, i32 step, bool shift = false) {
    PageInput in = {};
    in.kind = PageInput::turn, in.encoder = encoder, in.step = step, in.shift = shift;
    return in;
}
static bool fake_name(u32, u32, char* text, u32) { return text[0] = 0, false; }
static const Names names = {fake_name};
static Heard heard;
static const u32 kSongEntries = 96, kSceneLen = 32, kPinned = 40;

// A position in entry `e`, `step` steps in, playing the scene the song has there.
static SongPosition at(const SongShape& song, u32 e, u32 step, bool last_stops = false) {
    SongPosition p = {};
    p.playing = true, p.song = 0, p.entry = e, p.next_entry = e + 1;
    p.position = step * 480, p.length = song.steps[e] * 480;
    p.scene = song.scene[e], p.scene_before = e ? song.scene[e - 1] : kNoScene;
    p.stops = last_stops && e + 1 == song.entries;
    return p;
}

int main() {
    static SongShape song = {};
    song.entries = kSongEntries;
    for (u32 i = 0; i < kSongEntries; ++i) song.scene[i] = static_cast<u8>(i % 12), song.steps[i] = kSceneLen;
    static Store store;
    static VariationPage page;
    Around a = {};
    a.song = true, a.song_index = 0, a.entries = kSongEntries, a.entry = kPinned;
    a.scene_before = song.scene[kPinned - 1], a.scene_after = song.scene[kPinned];
    a.steps_before = kSceneLen, a.steps_after = kSceneLen, a.shape = &song;
    page.open(store, a, heard, &names);
    auto give = [&](const PageInput& in) { require(!page.input(store, in, &names), "the page never closes itself"); };
    require(page.lowest_start() == -static_cast<i32>(kPinned * kSceneLen), "the start may reach the song's start, 40 scenes back (1280 steps)");

    // The first variation: a rise on lane A (gesture 4, the default for a new one). Grow it on the page with Shift +
    // encoder 2, as a user does: the bar lengths the song holds before the join, then the scene starts and ends.
    // Up to 1280 steps the window ends on the join (the end kept); a bar over it, the scene ends open.
    for (u32 i = 0; i < 400 && page.draft().length < 1280; ++i) give(turn(1, 1, true));
    require(page.draft().length >= 1024 && page.draft().offset == -static_cast<i32>(page.draft().length), "grown backwards from the join to 1024 steps or more");
    for (u32 i = 0; i < 400 && page.draft().offset + static_cast<i32>(page.draft().length) <= 0; ++i) give(turn(0, 1));
    // Over the join (one detent right), then up the list of lengths: the bar lengths, the scene ends (keeping the start),
    // the scene starts (keeping the end, moving the start back), to the longest the song allows: Shift + encoder 2 until it
    // stops changing. That is the window from the song's first step to its last.
    give(turn(0, 1));
    require(page.draft().offset + static_cast<i32>(page.draft().length) > 0, "the window crosses the join");
    for (u32 i = 0; i < 600; ++i) {
        const u32 before = page.draft().length;
        give(turn(1, 1, true));
        if (page.draft().length == before) break;
    }
    const Draft& d = page.draft();
    require(d.offset == -static_cast<i32>(kPinned * kSceneLen) && d.length == kSongEntries * kSceneLen, "the window spans the whole song: 3072 steps from entry 0");
    require(d.length > 1024 && d.length <= kMaxLength, "longer than 64 bars, within the new ceiling");
    require(d.reach == kSongEntries - 1 - kPinned && d.back == kPinned && Store::valid(d), "a join each side: 55 forward, 40 back (build 064: entry 0 too, the window starts on its start; was 39)");
    require(store.count() == 1 && store[0].reach == 55 && store[0].back == 40 && store[0].length == 3072 && store[0].offset == -1280, "stored whole");
    require(store.version() == 12, "version 12");
    // Its records are the song's joins.
    u32 scene = 0, start = 0;
    store.reach(store[0], 55, &scene, &start);
    require(scene == song.scene[95] && start == 55 * kSceneLen, "the last forward record: entry 95, 1760 steps on");
    store.back(store[0], 40, &scene, &start);
    require(scene == song.scene[0] && start == 40 * kSceneLen, "the last backward record (build 064): entry 0, 1280 steps before the join");
    store.back(store[0], 39, &scene, &start);
    require(scene == song.scene[1] && start == 39 * kSceneLen, "the one before it: entry 1, 1248 steps before the join");

    // Played back: in every entry, 0 to 95, the window is at the song's own step (it starts with the song). Build 064 (// a window starting on the song's start left its whole first scene silent): entry 0 is reached now, from its first step.
    u32 w = 0, before = 0, after = 0;
    for (u32 e = 0; e < kSongEntries; ++e) {
        for (u32 step = 0; step < kSceneLen; step += kSceneLen - 1) {
            const bool ok = window_at(store[0], at(song, e, step), &w, &store);
            require(ok, "plays in every entry, the first included (build 064)");
            require(w == (e * kSceneLen + step) * 480, "at the song's own step: continuous across all the joins");
        }
    }
    require(window_at(store[0], at(song, 0, 0), &w, &store) && w == 0, "build 064: the song's very first step is the window's first");
    require(window_at(store[0], at(song, 0, 31), &before, &store) && window_at(store[0], at(song, 1, 0), &after, &store) && after - before == 480,
            "build 064: continuous across the first join");
    require(window_at(store[0], at(song, 1, 0), &w, &store) && w == 32 * 480, "near the start: entry 1, the window's 32nd step");
    require(window_at(store[0], at(song, 95, 31, true), &w, &store) && w == 3071 * 480, "the song's last step is the window's last");
    require(!window_at(store[0], at(song, 95, 31, true), &w) , "(without the store the reach is not looked at)");
    require(window_at(store[0], at(song, 40, 0), &w, &store) && w == 1280 * 480, "the join's own entry: at 1280 steps");
    require(window_at(store[0], at(song, 39, 31), &w, &store) && w == 1279 * 480, "the entry before the join: the live section length");
    require(window_at(store[0], at(song, 70, 16), &w, &store) && w == (70 * 32 + 16) * 480, "the middle of the reach: entry 70");
    require(window_at(store[0], at(song, 20, 16), &w, &store) && w == (20 * 32 + 16) * 480, "the middle of the back: entry 20");
    // A wrong scene in a far entry stays silent: entry 70 playing a scene the record does not name.
    SongPosition wrong = at(song, 70, 5);
    wrong.scene = static_cast<u32>((song.scene[70] + 1) % 12);
    require(!window_at(store[0], wrong, &w, &store), "a wrong scene in a reach entry: silent");
    wrong = at(song, 10, 5);
    wrong.scene = static_cast<u32>((song.scene[10] + 1) % 12);
    require(!window_at(store[0], wrong, &w, &store), "a wrong scene in a back entry: silent");

    // A new variation's lane A is a break (it holds, then falls to nothing): its line (256 steps) is stretched over the whole
    // 3072-step window (stagger.hpp), so it starts as set, is part way down at the song's middle (entry 48) and ends at the
    // bottom, and never climbs back across any of the joins in between.
    require(store[0].lanes[0].target != kNoTarget && store[0].lanes[0].count >= 2, "lane A holds a line");
    const u32 word = store[0].lanes[0].target;
    Targets tg;
    u32 previous = 0, value = 0;
    bool first = true;
    for (u32 e = 1; e < kSongEntries; ++e) {
        for (u32 step = 0; step < kSceneLen; step += 8) {
            targets(store, at(song, e, step, true), &tg);
            bool driven = false;
            for (u32 t = 0; t < kTracks && !driven; ++t) driven = tg.find(t, word, &value);
            require(driven, "driven at every step sampled");
            require(first || value <= previous, "the break never climbs back across a join");
            previous = value, first = false;
        }
    }
    targets(store, at(song, 48, 0), &tg);
    u32 mid = 0, top = 0, bottom = 0;
    require(tg.find(0, word, &mid) || tg.find(1, word, &mid) || tg.find(2, word, &mid), "driven in the middle");
    targets(store, at(song, 95, 31, true), &tg);
    for (u32 t = 0; t < kTracks; ++t)
        if (tg.find(t, word, &top)) break;
    targets(store, at(song, 1, 0), &tg);
    for (u32 t = 0; t < kTracks; ++t)
        if (tg.find(t, word, &bottom)) break;
    require(bottom == 128 * 256 && top == 0 && mid > top && mid <= bottom, "the break runs across the whole song: as set at the start, nothing at the end");
    std::puts("variations spanning: ok");
    return 0;
}
