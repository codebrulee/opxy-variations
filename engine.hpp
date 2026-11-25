// Variations: a lane's engine control (build 025, format version 6). The
// OP-XY's engines name their four M1 controls their own way ("tone", "env",
// "shape", "tremolo" on one synth; others on the next), and the Synthdawg
// guide calls them "parameter 1".."parameter 4" whatever the engine. A lane
// moves one of two kinds:
//
//   - a slot, "param 1".."param 4" (target word 0..3, no name): that slot on
//     every lane track at once, whatever each engine calls it;
//   - a name, "tone" (target word: the slot it was found in, and the name
//     stored): on each lane track whose engine has a control of that name,
//     in whatever slot it has it; other tracks are left alone.
//
// Either way a track is left alone where the slot is unsafe to ramp or not
// there (usable()): no name or a stock placeholder ("param1", "param 1", the
// SDK's position "m1 1"), no control at all (an SDK synth's choice or absent
// slot: the SDK refuses those), or parameter 4 of a drum or sampler engine
// (play mode, sample end: a switch or a hit's bounds, not a level).
#pragma once
#include <opxy/extension.hpp>

namespace variations {
using opxy::i32;
using opxy::u32;
using opxy::u8;
// A name as an engine gives it: track_label_capacity holds every name.
const u32 kNameCapacity = opxy::extensions::track_label_capacity;
const u32 kNameChars = kNameCapacity - 1;  // at most 15 characters (format version 6)
// The slots the page offers by position ("param 1".."param 4"): the M1 page's
// four encoders, always offered. Build 030: slots 5..8 (Shift) also by
// position, while some lane track has a control there (page.hpp list_targets).
const u32 kParamSlots = 4;
// Parameter 4 (word 3): play mode on Drum engines, the sample's end on the
// Sampler and Multisampler (Synthdawg's OP-XY guide, "Lockable parameters").
const u32 kParamFour = 3;

// What placing an engine control needs of the SDK: the track's name for a
// word (Actions::track_target_label; false for no control there), and which
// tracks are known to be percussion (Drum, Drumline: the heard notes' flag).
struct Engines {
    bool (*label)(u32 track, u32 word, char* text, u32 capacity);
    u8 percussion;
};

namespace engine {
inline bool same(const char* a, const char* b) {
    u32 k = 0;
    while (a[k] && a[k] == b[k]) ++k;
    return a[k] == b[k];
}
inline void copy(char* to, const char* from, u32 capacity) {
    u32 n = 0;
    for (; from[n] && n + 1 < capacity; ++n) to[n] = from[n];
    to[n] = 0;
}
// A slot the engine leaves unnamed: empty, stock's "param1".."param8" (the
// SDK hides those already) or "param 1" (another stock spelling), or the
// SDK's position for a nameless slot ("m1 1".."m1 8").
inline bool unnamed(const char* s) {
    if (!s[0]) return true;
    const char* p = "param";
    u32 k = 0;
    while (p[k] && s[k] == p[k]) ++k;
    if (!p[k]) {
        if (s[k] == ' ') ++k;
        return s[k] >= '0' && s[k] <= '9';
    }
    return s[0] == 'm' && s[1] == '1' && s[2] == ' ' && s[3] >= '1' && s[3] <= '8' && !s[4];
}
// Parameter 4 names a drum or sampler engine gives it, not to ramp.
inline bool unsafe_four(const char* s) {
    static const char* const names[] = {"play mode", "playmode", "mode", "end", "sample end"};
    for (const char* n : names)
        if (same(s, n)) return true;
    return false;
}
// Whether `track` may move engine slot `word` (named `label` there): named,
// and not parameter 4 of a drum or sampler engine (a percussion track, or a
// name that says so).
// Build 032 (2026-11-23): a drum or sampler engine's parameter 4 is
// offered again ("reverse", play mode, sample end). 025-031 hid it as unsafe
// to ramp, a guess from the guide, never heard on the device, and never told
// to the player. It sweeps like any control: a switch flips where the line crosses.
inline bool safe(u32 track, u32 word, const char* label, u8 percussion) {
    (void)track, (void)word, (void)percussion;
    return !unnamed(label);
}
// The track's name for slot `word` when it may be moved (into `text`), else false.
// Build 034: a control the engine leaves unnamed (the SDK's "m1 N") is a
// control all the same: usable, `text` empty (the page calls it "param N").
// 025-033 left it alone, so a Drumline's M1 controls were never offered on
// its own track (2026-11-23). Only no control at all (the SDK's false)
// is unusable.
inline bool usable(const Engines& e, u32 track, u32 word, char* text) {
    text[0] = 0;
    if (!e.label || !e.label(track, word, text, kNameCapacity)) return text[0] = 0, false;
    if (unnamed(text)) text[0] = 0;
    return true;
}
// Whether any name can be read on `track` now (the volume's fixed label:
// every track names it); outside a callback none can.
inline bool readable(const Engines& e, u32 track) {
    char text[kNameCapacity];
    return e.label && e.label(track, 15, text, sizeof text);
}
// The word a lane's engine target moves on `track`, or -1 for none: `slot`
// (0..7) alone when `name` is empty, else the slot whose name is `name`.
// Null `e` (no names to read: the notes' pass only asks the volume and the
// notes): the slot itself, and nothing for a name. Where no name can be read
// at all (not even the volume's) the slot is moved as before build 025.
inline i32 word(const Engines* e, u32 track, u32 slot, const char* name) {
    if (slot >= opxy::extensions::engine_words) return -1;
    if (!e) return name && name[0] ? -1 : static_cast<i32>(slot);
    char text[kNameCapacity];
    if (!name || !name[0]) {
        if (usable(*e, track, slot, text)) return static_cast<i32>(slot);
        return readable(*e, track) ? -1 : static_cast<i32>(slot);
    }
    for (u32 w = 0; w < opxy::extensions::engine_words; ++w)
        if (usable(*e, track, w, text) && text[0] && same(text, name)) return static_cast<i32>(w);
    return -1;
}
}
}
