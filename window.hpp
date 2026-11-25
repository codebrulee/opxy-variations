// Variations: where the song is, and what each line asks for there.
// A variation is pinned to the join at the start of entry `entry` of song
// `song`; its window is [offset, offset + length) steps from that join, within
// the section before the join and the section after it, and (build 047, format
// version 8) on past that into the entries after it, as far as its reach list
// says (notes/variations-format.md), and (build 058, version 10) back across the entries before it
// as far as its backward list says. All the cases are found from the scene
// clock of the entry now playing, so no other scene's length is needed:
//
//   playing the entry before E (the one whose next join starts E):
//                        at = position - (length_now + offset * 480)
//   playing entry E:     at = position - offset * 480
//   playing entry E + j (1 <= j <= reach; its record's start_j is the steps
//                        from E's join to the join into E + j):
//                        at = position + (start_j - offset) * 480
//   playing entry E - j (2 <= j <= back; build 058; its record's start_j is the steps
//                        from the join into E - j to E's join):
//                        at = position - (start_j + offset) * 480
//                        (entry E - 1 is the "entry before" case above, from the live scene length)
//
// with `at` the time into the window in scene clock units (480 a step);
// the window plays while 0 <= at < length * 480. A section shorter than the
// part of the window before the join starts it part-way through.
#pragma once
#include "store.hpp"

namespace variations {
// The song as the scene clock stands after a pass (from the SDK's song
// position): which song and entry play, and the time into and length of the
// entry's scene, in 480 a step.
struct SongPosition {
    bool playing;  // the scene clock runs and song playback is on
    u32 song, entry, next_entry;  // next_entry: the entry the next join starts
    u32 position, length;
    u32 scene;  // the scene playing now (a cued scene plays in place of the song's)
    u32 scene_before;  // the scene the entry before this one played, when the song came from it; else kNoScene
    // Playback (re)started at this entry: Play, a jump to it (the song page's
    // cursor), or the song's start (entry 0, also when the song loops round).
    bool restarted;
    // The song stops at the next boundary instead of going on: its last
    // entry, in a song that doesn't loop. No join follows, so nothing plays
    // before one, but for a window at the song's end (build 025): one pinned
    // to the join into entry 1 that ends at or before it, which the page puts
    // there on a song that doesn't loop (its "scene n > end").
    bool stops;
};

// The time into v's window now, or false when it doesn't play here. A
// variation whose scenes are known plays only while the scene playing is the
// one it expects on that side of its join: after a song edit not yet seen by
// repin (store.hpp; it needs read_song, which only the page has), it stays
// silent rather than play at the wrong join. In its entry, the scene the song
// came from must be its scene before, when both are known; but where playback
// (re)started at this entry (Play, a jump, the song's start) there is nothing
// to have come from, and a window that begins at or after the join plays from
// the moment playback starts there (a fade-in at the song's start, on Play and
// as the song loops round). Unplaced variations never play.
//
// window_time: the same, signed, also `lead` ticks before the window starts
// (into < 0) and `tail` ticks after it ends (into >= its span): the eases
// (targets(), below). Before a window that starts on or after its join the
// lead may fall in the section before the join.
//
// Build 047: `store` holds the reach records of a reaching variation (null: its
// reach is not looked at, as before version 8). Build 058: and its backward records.
inline bool window_time(const Variation& v, const SongPosition& s, u32 lead, u32 tail, i32* time, const Store* store = nullptr) {
    if (!s.playing || s.song != v.song || v.unplaced) return false;
    const i32 offset = v.offset * static_cast<i32>(kStepTicks);
    const i32 span = static_cast<i32>(v.length * kStepTicks);
    i32 into;
    if (s.entry == v.entry) {
        // (kNoScene: the scene the next pass starts, not known yet: ahead())
        if (v.scene_after != kNoScene && s.scene != kNoScene && s.scene != v.scene_after) return false;
        const bool arrived = v.scene_before == kNoScene || s.scene_before == kNoScene || s.scene_before == v.scene_before ||
                             (s.restarted && v.offset >= 0);
        if (!arrived) return false;
        into = static_cast<i32>(s.position) - offset;
    } else if (store && s.entry > v.entry && s.entry - v.entry <= v.reach) {
        // Build 047: a later entry the window runs on into. Its record's scene, when it and the scene
        // playing are known, must match (kNoScene from ahead(): the record is trusted). No `arrived`
        // check: Play, or a jump into a middle entry, plays from there.
        u32 scene = 0, start = 0;
        store->reach(v, s.entry - v.entry, &scene, &start);
        if (scene != kNoScene && s.scene != kNoScene && s.scene != scene) return false;
        into = static_cast<i32>(s.position) + (static_cast<i32>(start) - v.offset) * static_cast<i32>(kStepTicks);
    } else if (store && s.entry < v.entry && v.entry - s.entry >= 2 && v.entry - s.entry <= v.back) {
        // Build 058 (version 10): an earlier entry, two or more before the join, the window's start reaches
        // back into. Its record's scene, when it and the scene playing are known, must match (as above).
        // The join into E is start_j steps on from this entry's start, so the window's start (offset
        // from that join) is at start_j + offset steps into it. No `arrived` check, as above.
        // Entry E - 1 is the branch below, from the live section length.
        u32 scene = 0, start = 0;
        store->back(v, v.entry - s.entry, &scene, &start);
        if (scene != kNoScene && s.scene != kNoScene && s.scene != scene) return false;
        into = static_cast<i32>(s.position) - (static_cast<i32>(start) + v.offset) * static_cast<i32>(kStepTicks);
    } else if ((v.offset < 0 || lead) && s.next_entry == v.entry &&
               (!s.stops || (v.entry == 0 && v.offset + static_cast<i32>(v.length) <= 0))) {  // the section before the join
        if (v.scene_before != kNoScene && s.scene != v.scene_before) return false;
        into = static_cast<i32>(s.position) - (static_cast<i32>(s.length) + offset);
    } else {
        return false;
    }
    if (into < -static_cast<i32>(lead) || into >= span + static_cast<i32>(tail)) return false;
    *time = into;
    return true;
}
inline bool window_at(const Variation& v, const SongPosition& s, u32* at, const Store* store = nullptr) {
    i32 into;
    if (!window_time(v, s, 0, 0, &into, store)) return false;
    *at = static_cast<u32>(into);
    return true;
}

// The song as the next pass will find it, `ticks` on (this pass's advance):
// the track values are sent one pass early, so they are in place before that
// pass's notes start (stock starts a step's notes in its controller pass
// before sequencer_pass sends anything; a build's first note was heard for a
// moment at full volume, build 014). Over a scene boundary: the next entry
// from its start, its scene `next_scene` (a cued one) or not known (kNoScene:
// the scene guard then trusts the variation's own scene after), come from the
// scene now playing; the song's start (entry 0) counts as a restart, as the
// pass will find it. Nothing ahead where the song stops at the boundary, and
// no join after the next entry is known (its length isn't).
const u32 kNoEntry = 0xffffffffu;
inline SongPosition ahead(const SongPosition& s, u32 ticks, u32 next_scene = kNoScene) {
    SongPosition n = s;
    if (!s.playing || !ticks) return n;
    const u32 to = s.position + ticks;
    if (to < s.length) {
        n.position = to;
        return n;
    }
    if (s.stops) return n;
    n.entry = s.next_entry;
    n.next_entry = kNoEntry;
    n.position = to - s.length;
    n.length = 0;
    n.scene = next_scene;
    n.scene_before = s.scene;
    n.restarted = n.entry == 0;
    n.stops = false;
    return n;
}

// Where Play will start, while stopped (build 016): entry `entry` of `song`
// from its start, as a restart (window_at), its scene not known (the scene
// guard trusts the variation's own) and nothing known after it. Play's first
// notes, which still see the stopped position, are judged there (a note on a
// track a lane holds silent is dropped); nothing is sent there while stopped
// (build 018: the keys sound as set), only in a count-in.
inline SongPosition play_start(u32 song, u32 entry) {
    SongPosition p = {};
    p.playing = true;
    p.song = song, p.entry = entry, p.next_entry = kNoEntry;
    p.position = 0, p.length = 0;
    p.scene = kNoScene, p.scene_before = kNoScene;
    p.restarted = true;
    p.stops = false;
    return p;
}

// Whether a track's written note should be dropped: its volume is driven at
// (or within kSilent of) the bottom where it starts, so it would only be heard
// for the moment before the value reaches the audio board.
const u32 kSilent = 2 << 8;  // Q8: under 1 % of as set

// What the lanes ask now: per track and word a Q8 value (0..255 * 256, the
// middle "as set"), and per track the notes line's. Where two lanes (of one
// variation, or of two whose windows overlap) drive the same word of a track,
// the one met first decides: variations in store order, lane A before B before C.
//
// This list holds what the lanes *ask*, not what is sent: the mixer sends a
// word only when its value changes, which is about three a pass and never more
// than the SDK's budget for one callback (a word whose send that budget refuses
// goes on the next pass instead, 1.45 ms later). So the list is sized to the
// most a song can ask for rather than to that budget: a track's word may be
// asked for only once, so eight tracks of the SDK's 28 drivable words is every
// distinct ask there is, and the list cannot fill however many variations
// overlap.
const u32 kMaxDrives = kTracks * opxy::extensions::track_target_count;  // 224: every ask there is
// Words are the SDK's indices, under 58. One bit each, so the duplicate check
// that runs for every lane and track is a test rather than a walk of the list.
const u32 kWordBits = 64;
struct Drive {
    u8 track, word;
    u32 value;
};
struct Targets {
    u32 count;
    Drive drive[kMaxDrives];
    u32 notes[kTracks];
    bool notes_set[kTracks];
    u32 asked[kTracks][kWordBits / 32];
    // Whether a track's word has been asked for already.
    bool taken(u32 track, u32 word) const { return asked[track][word >> 5] >> (word & 31) & 1; }
    // Add a track's word, unless it is taken or the list is full (it cannot be).
    void add(u32 track, u32 word, u32 value) {
        if (taken(track, word) || count >= kMaxDrives) return;
        asked[track][word >> 5] |= 1u << (word & 31);
        drive[count++] = Drive{static_cast<u8>(track), static_cast<u8>(word), value};
    }
    void clear() {
        count = 0;
        for (u32 t = 0; t < kTracks; ++t) {
            notes_set[t] = false, notes[t] = 0;
            asked[t][0] = asked[t][1] = 0;
        }
    }
    // The value asked of a track's word, or false. Not on the per-pass path:
    // the duplicate check uses taken(), this is for a single lookup.
    bool find(u32 track, u32 word, u32* value) const {
        if (!taken(track, word)) return false;
        for (u32 i = 0; i < count; ++i)
            if (drive[i].track == track && drive[i].word == word) return *value = drive[i].value, true;
        return false;
    }
};

// The eases (build 016): `ease` ticks (a few passes) before a window, a lane's
// value goes from as set to the window's start value, arriving there as the
// window starts, so a window that starts at silence (a fade in on the song's
// start, as the song loops round) doesn't cut the sound in one block. After
// a window whose end value is far from as set (kEaseJump), it goes back to as
// set over as long (build 074: longer, see out_length). Not after break and drop: their comebacks are sudden on
// purpose (their silenced notes were dropped, so nothing clicks). Not on the
// notes. A window's own values come first; an ease drives only a word no
// window does.
const u32 kEaseJump = 32 << 8;  // Q8: an eighth of the range
inline bool eases_out(u32 gesture) { return gesture != 2 && gesture != 3; }  // not break, drop
// Build 074: the climb back out of a window gets its own, longer length (`ease_out`
// ticks at most; 0: as long as the ease in). The ease in only has to avoid cutting
// the sound in one block; the climb out is heard on whatever still rings (a release
// tail), where a full-scale volume step in a few ms is a click. The length scales
// with the jump: ease + (ease_out - ease) * jump / kMiddle (a fade to silence, the
// biggest jump, takes all of ease_out; a step just over kEaseJump, a quarter of the
// extra), so a small step back is not dragged out.
inline u32 out_length(u32 ease, u32 ease_out, u32 jump) {
    if (ease_out <= ease) return ease;
    // 32-bit throughout, deliberately: 64-bit arithmetic pulls in GCC's helpers, and this compiler profile carries
    // no authenticated arithmetic runtime, so the firmware build refuses it ("no authenticated arithmetic runtime",
    // docs/toolchain.md). The host tests compile for x86 and cannot see that. It fits easily: ease_out - ease is a
    // few hundred ticks and the jump at most kMiddle, so the product is about 21 million against u32's 4.29 billion.
    const u32 room = ease_out - ease, by = jump > kMiddle ? kMiddle : jump;
    return ease + room * by / kMiddle;
}
inline u32 eased(u32 from, u32 to, u32 done, u32 ease) {
    const i32 a = static_cast<i32>(from), z = static_cast<i32>(to);
    return static_cast<u32>(a + (z - a) * static_cast<i32>(done) / static_cast<i32>(ease));
}
// The word lane `n` of v moves on `track` (engine.hpp: an engine control by
// slot or by name, resolved on each track's own engine; -1 for none there),
// or its target as it is.
inline i32 lane_word(const Store& store, const Variation& v, u32 n, u32 track, const Engines* engines) {
    const u32 target = v.lanes[n].target;
    if (target >= opxy::extensions::engine_words) return static_cast<i32>(target);
    char name[kNameCapacity];
    store.name(v, n, name);
    return engine::word(engines, track, target, name);
}
inline void ease_targets(const Store& store, const SongPosition& s, u32 ease, Targets* out, const Engines* engines, u32 ease_out = 0) {
    if (ease_out < ease) ease_out = ease;
    // Build 074: every lead-in first, then the climbs out. A word goes to whichever ease claims it first (`find`
    // below), and that used to be whichever variation came first in the store - so a variation climbing out of a
    // window could hold a word that the next variation was trying to lead into. When that next window then opened,
    // its own value took the word at once, and the gap between the climb's value and the window's start value was a
    // jump: the very click this build is removing, at a different seam. Build 074's longer climb widened the overlap
    // from about 9 ms to as much as 46 ms, so it had to be settled. A lead-in wins because it is going where the
    // sound is about to go and it arrives exactly on the window's start value, so nothing jumps when the window opens.
    for (u32 phase = 0; phase < 2; ++phase)
    for (u32 i = 0; i < store.count(); ++i) {
        const Variation& v = store[i];
        const i32 span = static_cast<i32>(v.length * kStepTicks);
        i32 into;
        if (!window_time(v, s, ease, ease_out, &into, &store) || (into >= 0 && into < span)) continue;
        const bool before = into < 0;
        if (before != (phase == 0)) continue;  // phase 0: the lead-ins; phase 1: the climbs out
        for (u32 n = 0; n < kLanes; ++n) {
            const u32 target = v.lanes[n].target;
            if (target == kNoTarget || target == kTargetNotes || is_stutter(v.lanes[n].gesture) || v.lanes[n].muted) continue;
            if (!before && !eases_out(v.lanes[n].gesture)) continue;
            const Lane& l = v.lanes[n];
            u32 count;
            const Point* p = store.points(v, n, &count);
            if (!p) continue;
            for (u32 t = 0; t < kTracks; ++t) {
                if (!(l.selection >> t & 1)) continue;
                const i32 word = lane_word(store, v, n, t, engines);
                if (word < 0 || out->taken(t, static_cast<u32>(word))) continue;
                const u32 shift = stagger(l.gesture, l.amount, l.curve, l.selection, t, v.length);
                const u32 edge = value_late(p, count, before ? 0 : static_cast<u32>(span - 1), shift, 1, v.length);  // build 049: length for the stretch
                u32 value;
                if (before) {
                    value = eased(kMiddle, edge, static_cast<u32>(into + static_cast<i32>(ease)), ease);
                } else {
                    const u32 jump = edge > kMiddle ? edge - kMiddle : kMiddle - edge;
                    if (jump < kEaseJump) continue;
                    const u32 length = out_length(ease, ease_out, jump);
                    const u32 done = static_cast<u32>(into - span + 1);
                    if (done > length) continue;  // arrives at as set on done == length, exactly
                    value = eased(edge, kMiddle, done, length);
                }
                out->add(t, static_cast<u32>(word), value);
            }
        }
    }
}

// `engines` places the lanes' engine controls on each track (engine.hpp);
// null: by slot, and a control by name moves nothing (the notes' check, which
// asks only the volume and the notes).
inline void targets(const Store& store, const SongPosition& s, Targets* out, u32 ease = 0, const Engines* engines = nullptr, u32 ease_out = 0) {
    out->clear();
    for (u32 i = 0; i < store.count(); ++i) {
        const Variation& v = store[i];
        u32 at;
        if (!window_at(v, s, &at, &store)) continue;
        for (u32 n = 0; n < kLanes; ++n) {
            const u32 target = v.lanes[n].target;
            // Build 047: stutter's line is its density (retrigger.hpp), not a value to drive:
            // its natural target is the notes, which it must not move.
            // Build 047: a muted lane plays nothing.
            if (target == kNoTarget || is_stutter(v.lanes[n].gesture) || v.lanes[n].muted) continue;
            const Lane& l = v.lanes[n];
            u32 count;
            const Point* p = store.points(v, n, &count);
            if (!p) continue;
            // Every track of the lane follows its one line (build and break
            // one after another: stagger.hpp).
            for (u32 t = 0; t < kTracks; ++t) {
                if (!(l.selection >> t & 1)) continue;
                // Build 044: gate's stored bar repeats every bar of the window.
                const u32 rate = l.gesture == kGestureGate ? l.rate * stutter_scale(v.length) : l.rate;
                const u32 value = value_late(p, count, at, stagger(l.gesture, l.amount, l.curve, l.selection, t, v.length), rate, v.length);
                if (target == kTargetNotes) {
                    if (out->notes_set[t]) continue;
                    out->notes[t] = value;
                    out->notes_set[t] = true;
                } else {
                    const i32 word = lane_word(store, v, n, t, engines);
                    if (word >= 0) out->add(t, static_cast<u32>(word), value);
                }
            }
        }
    }
    if (ease) ease_targets(store, s, ease, out, engines, ease_out);
}
}
