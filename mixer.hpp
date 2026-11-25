// Variations: the lanes' track words as track values (probes 085-090; the
// SDK's track_targets). Each pass, every word a lane drives on a track is
// turned into its value and sent when it changed; a word no lane drives any
// more, or every word on Stop or a project change, goes back to the GUI's own
// value (stock re-sends its value only when that value changes, so it must be
// put back by hand). When the GUI's own value changes under a word we hold,
// stock has just sent it (its publish sends every change), so ours goes again
// even where it is the same; and refresh() sends all of ours again (Play).
//
// A lane's value is 0..255 with 128 "as set": below it the word moves from
// the GUI's value towards the knob's counter-clockwise end (0 at the bottom),
// above it towards the clockwise end (255 at the top). The knob's direction is
// the SDK's: a release (-1) is stored mirrored, so its clockwise end is word 0.
// The cutoff stays within kClosed..kOpen (closed, not choked: probe 088), and
// the M3 filter's words move only where M3 is on and not the dynamic type.
#pragma once
#include "window.hpp"

namespace variations {
const i32 kFull = 0x7fffffff;
const i32 kClosed = 0x0a000000;  // about 8 %: closed, not choked (probe 088)
const i32 kOpen = 0x70000000;    // about 88 %
const u32 kFilterDynamic = 18;
const u32 kFilterFirst = 16, kFilterLast = 19;  // cutoff, resonance, env amount, key track

// What the mixer needs of the SDK, so tests can fake it.
struct TrackIo {
    bool (*read)(u32 track, u32 word, i32* value);
    bool (*filter)(u32 track, u32* type, bool* on);
    bool (*send)(u32 track, u32 word, i32 value);
    // Whether the track's engine has engine control `word` (0..7); null: every
    // track has every slot. A track without it is left alone (build 024).
    bool (*has)(u32 track, u32 word);
};

// from + (to - from) * k / 65280 (k Q8, 0..255 * 256), either direction, in 32 bits.
inline i32 toward(i32 from, i32 to, u32 k) {
    const u32 span = to >= from ? static_cast<u32>(to - from) : static_cast<u32>(from - to);
    // span * k / 65280 with span = hi * 65536 + lo: hi * k * 65536 / 65280 = hi * k * (1 + 1/255).
    const u32 hi = span >> 16, lo = span & 0xffff;
    const u32 step = hi * k + hi * k / 255 + lo * k / 65280;
    return to >= from ? from + static_cast<i32>(step) : from - static_cast<i32>(step);
}

// The knob's direction for a word: +1, or -1 where stock stores it mirrored.
inline i32 direction(u32 word) {
    const opxy::extensions::TrackTarget* t = opxy::extensions::track_target(word);
    return t ? t->direction : 1;
}

// A lane value (Q8) on the GUI's own value of `word`: the word to send.
inline i32 word_value(u32 word, u32 v, i32 own) {
    if (own < 0) own = 0;
    const i32 low = word == kCutoff ? kClosed : 0, high = word == kCutoff ? kOpen : kFull;
    const bool up = direction(word) > 0;
    // The knob's counter-clockwise and clockwise ends, as words.
    const i32 bottom = up ? low : high, top = up ? high : low;
    if (v < kMiddle) {
        if (up ? own <= low : own >= high) return own;  // already past that end
        return toward(bottom, own, v * 2 > 0xff00 ? 0xff00 : v * 2);
    }
    if (up ? own >= high : own <= low) return own;
    return toward(own, top, (v - kMiddle) * 0xff00 / (0xff00 - kMiddle));
}

class Mixer {
public:
    // Every pass: send what the targets ask; give back what they no longer do.
    void apply(const Targets& t, const TrackIo& io) {
        for (Slot& s : slots_) s.wanted = false;
        for (u32 i = 0; i < t.count; ++i) {
            const Drive& d = t.drive[i];
            if (d.word < opxy::extensions::engine_words && io.has && !io.has(d.track, d.word)) continue;
            Slot* s = find(d.track, d.word, true);
            if (!s) continue;
            s->wanted = true;
            i32 own;
            if (!io.read(d.track, d.word, &own)) continue;
            bool driven = true;
            if (d.word >= kFilterFirst && d.word <= kFilterLast) {
                u32 type;
                bool on;
                driven = io.filter(d.track, &type, &on) && on && type != kFilterDynamic;
            }
            send(*s, driven ? word_value(d.word, d.value, own) : own, own, io);
        }
        for (Slot& s : slots_) {
            if (!s.used || s.wanted) continue;
            i32 own;
            if (io.read(s.track, s.word, &own)) send(s, own, own, io);
            if (!s.ours) s.used = false;
        }
    }
    // Stop or another project: every word back to the GUI's at once.
    void release(const TrackIo& io) {
        Targets none = {};
        apply(none, io);
    }
    // Every value of ours goes again on the next apply, the same or not: in
    // case stock sent its own over them (Play, before its first notes).
    void refresh() {
        for (Slot& s : slots_)
            if (s.used && s.ours) s.stale = true;
    }
    // A project change before anything could be sent: forget what the DSP held.
    void forget() {
        for (Slot& s : slots_) s = Slot();
    }
    bool ours(u32 track, u32 word) const {  // for tests
        for (const Slot& s : slots_)
            if (s.used && s.track == track && s.word == word) return s.ours;
        return false;
    }

private:
    struct Slot {
        bool used = false, wanted = false;
        u8 track = 0, word = 0;
        i32 sent = 0, own = 0;  // what we sent, and the GUI's own value then
        bool ours = false;  // the DSP holds a value of ours, not stock's
        bool stale = false;  // send ours again (refresh)
    };
    Slot* find(u32 track, u32 word, bool add) {
        Slot* free = nullptr;
        for (Slot& s : slots_) {
            if (s.used && s.track == track && s.word == word) return &s;
            if (!s.used && !free) free = &s;
        }
        if (!add || !free) return nullptr;
        *free = Slot();
        free->used = true, free->track = static_cast<u8>(track), free->word = static_cast<u8>(word);
        return free;
    }
    void send(Slot& s, i32 value, i32 own, const TrackIo& io) {
        if (!s.ours && value == own) return;  // stock's own value stands
        if (s.ours && value == s.sent && own == s.own && !s.stale) return;
        if (!io.send(s.track, s.word, value)) return;  // a project loads, or the pass's sends are spent: next pass
        s.sent = value, s.own = own, s.stale = false;
        s.ours = value != own;
    }
    // Words driven or still to give back: a pass drives at most kMaxDrives,
    // and those a lane let go wait here until given back.
    Slot slots_[2 * kMaxDrives];
};
}
