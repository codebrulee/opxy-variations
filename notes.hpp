// Variations: the notes line (probe 089, notes/variation-probe-5.md), on the
// SDK's sequenced_note event. A line value below the middle (128) thins a
// track, dropping a fixed share of its notes; above, it gets busier, adding
// one quieter hit midway to the next written note.
//
// The order is a pure function of the pattern, so every loop thins and fills
// the same way: notes are grouped per step (a chord stays whole; on drums per
// step and pad), each group valued by its metric weight (downbeats last) then
// its loudest note, ties broken by a fixed hash. At a thinning share t the
// lowest round(t * N) of the N groups don't start; at a busier share u the
// highest round(u * N) get an extra hit. Bar positions assume 16 steps a bar.
#pragma once
#include <opxy/extension.hpp>
#include "line.hpp"

namespace variations {
using opxy::extensions::Note;
using opxy::extensions::PatternNotes;
const u32 kBarSteps = 16;
const u32 kExtraVelocity = 70;  // % of the note's own

// Metric weight in a bar of 16: downbeat 4, half bar 3, beats 2, 8ths 1, 16ths 0.
inline u32 weight(u32 step) {
    const u32 s = step % kBarSteps;
    if (s == 0) return 4;
    if (s % 8 == 0) return 3;
    if (s % 4 == 0) return 2;
    return s % 2 == 0 ? 1 : 0;
}

// A fixed hash: the tie-break between groups of equal weight and velocity.
inline u32 mix(u32 x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    return x ^ (x >> 16);
}

inline u32 step_of(i32 position, u32 steps) {
    const u32 step = position < 0 ? 0 : (static_cast<u32>(position) + kStepTicks / 2) / kStepTicks;
    return steps ? step % steps : step;
}

// round(n * share / whole), share <= whole.
inline u32 part(u32 n, u32 share, u32 whole) { return (n * share + whole / 2) / whole; }

// One track's ranked pattern. Rebuilt only when its notes change.
class Ranking {
public:
    // Takes the track's notes; ranks them again if they changed. False when
    // there are none.
    bool refresh(const PatternNotes& p) {
        u32 signature = 2166136261u ^ p.steps ^ (p.percussion ? 0x80000000u : 0) ^ p.count;
        for (u32 i = 0; i < p.count; ++i) {
            const Note& n = p.notes[i];
            const u32 fields[] = {static_cast<u32>(n.position), n.length,
                                  static_cast<u32>(n.pitch) | static_cast<u32>(n.velocity) << 8};
            for (u32 f : fields) signature = (signature ^ f) * 16777619u;
        }
        if (!built_ || signature != signature_) build(p);
        signature_ = signature;
        built_ = true;
        return groups_ != 0;
    }
    void forget() { built_ = false; }

    u32 groups() const { return groups_; }
    // A note's rank, 0 = the first to drop, groups() - 1 = the last; -1 if it
    // isn't in the pattern.
    i32 rank(const Note& n) const {
        const u32 k = key(n);
        for (u32 r = 0; r < groups_; ++r)
            if (keys_[r] == k) return static_cast<i32>(r);
        return -1;
    }
    // The distance (480 a step) to the next written note in a later step (on
    // drums, of the same pad), wrapping round; the pattern's length if none.
    u32 gap(const Note& note) const {
        const i32 length = static_cast<i32>(steps_ * kStepTicks);
        const u32 step = step_of(note.position, steps_);
        i32 best = length;
        for (u32 i = 0; i < count_; ++i) {
            const Note& n = notes_[i];
            if ((drums_ && n.pitch != note.pitch) || step_of(n.position, steps_) == step) continue;
            i32 d = (n.position - note.position) % length;
            if (d < 0) d += length;
            if (d > 0 && d < best) best = d;
        }
        return static_cast<u32>(best);
    }
    u32 key_at(u32 r) const { return r < groups_ ? keys_[r] : ~0u; }  // for tests

private:
    u32 key(const Note& n) const { return step_of(n.position, steps_) * 128 + (drums_ ? n.pitch : 0); }
    bool before(u32 a, u32 b) const {
        if (value_[a] != value_[b]) return value_[a] < value_[b];
        const u32 ha = mix(keys_[a] + 1), hb = mix(keys_[b] + 1);
        return ha != hb ? ha < hb : keys_[a] < keys_[b];
    }
    void build(const PatternNotes& p) {
        steps_ = p.steps;
        drums_ = p.percussion;
        count_ = p.count;
        groups_ = 0;
        for (u32 i = 0; i < count_; ++i) {
            notes_[i] = p.notes[i];
            const u32 k = key(notes_[i]);
            u32 g = 0;
            while (g < groups_ && keys_[g] != k) ++g;
            const u32 value = weight(k / 128) << 8 | notes_[i].velocity;
            if (g == groups_) {
                keys_[groups_] = k;
                value_[groups_++] = value;
            } else if (value > value_[g]) {
                value_[g] = value;
            }
        }
        for (u32 i = 1; i < groups_; ++i)
            for (u32 j = i; j > 0 && before(j, j - 1); --j) {
                const u32 k = keys_[j], v = value_[j];
                keys_[j] = keys_[j - 1];
                value_[j] = value_[j - 1];
                keys_[j - 1] = k;
                value_[j - 1] = v;
            }
    }

    Note notes_[opxy::extensions::max_pattern_notes] = {};
    u32 keys_[opxy::extensions::max_pattern_notes] = {};
    u32 value_[opxy::extensions::max_pattern_notes] = {};
    u32 count_ = 0, steps_ = 0, groups_ = 0, signature_ = 0;
    bool drums_ = false, built_ = false;
};

// What the notes line does to one note.
struct NoteChoice {
    bool play;          // false: dropped
    bool extra;         // an extra hit `delay` after it (pattern ticks)
    u32 delay, velocity, length;  // the extra's
    u32 own_length;     // the note's own length (lowered to end before its extra)
};

// The choice for note n of rank r among `groups`, at line value v (Q8), with
// `gap` to the next note and the note's length and velocity as stock plays it.
inline NoteChoice choose(u32 v, u32 r, u32 groups, u32 gap, u32 length, u32 velocity) {
    NoteChoice c = {true, false, 0, 0, 0, length};
    if (v < kMiddle) {
        c.play = r >= part(groups, kMiddle - v, kMiddle);
        return c;
    }
    if (v == kMiddle || r < groups - part(groups, v - kMiddle, (255 << 8) - kMiddle)) return c;
    const u32 half = gap / 2;
    if (!half) return c;
    c.extra = true;
    c.delay = half;
    c.velocity = velocity * kExtraVelocity / 100;
    if (!c.velocity) c.velocity = 1;
    c.length = half - half / 8;
    if (c.own_length > c.length) c.own_length = c.length;  // ends before its extra starts
    if (c.length > length) c.length = length;
    return c;
}
}
