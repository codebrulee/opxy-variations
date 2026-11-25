// Variations: the project's variations, as saved (notes/variations-format.md).
// Holds the saved bytes (format version 12) and an index into them; load()
// checks the whole thing first and keeps nothing on any fault, as the JSON
// tool refuses. Version 1-4 bytes are rewritten as version 12 as they load;
// version 5 bytes are version 6's without a named engine control (build
// 025), version 6's are version 7's without a rate (flags bits 4-5 of a
// lane: log2 of how many times its line plays across the window, build 041),
// and version 7's are version 8's without a muted lane (flags bit 6) or a
// reach list (build 047), and version 8's are version 9's within the smaller
// ranges (build 049), and version 9's are version 10's without a backward
// reach list (build 058), and version 10's are version 11's with a lane C that
// is empty (build 060): those get the empty lane written in after their two,
// each variation's last seven bytes (a store too full to take them is kept as
// it is, as a version 1-4 one too full is). Version 11's are version 12's
// within the smaller ranges (build 063): nothing is written in, only the
// number changes.
//
// A variation is a window over the song (song, entry, offset, length, its
// scenes) and three lanes, A, B and C. A lane moves one target (a track word the
// SDK's track_targets lists, or the sequenced notes) on its own tracks, by a
// gesture: one line of points, 0..255 with 128 "as set", that all its tracks
// follow (version 5, build 024; version 4 had a line a track); build's and
// break's tracks follow it one after another (stagger.hpp). A lane's curve
// bends its gesture's shape (version 4). Version 6 (build 025): an engine
// control's lane may name the control it moves (engine.hpp). Version 7
// (build 041): a lane's rate, 1, 2, 4 or 8. Version 8 (build 047): a lane may
// be muted, and a window up to 256 steps may run on across later joins (its
// reach list). Version 9 (build 049): a window up to 1024 steps (64 bars) and
// a reach of up to 31 joins; a line has its own length, line_length() steps,
// stretched over a longer window (stagger.hpp value_late). Version 10 (build
// 058: "have another variation span the entire song, despite it being put
// at the first instance of scene 2"): a backward reach list, the mirror of the
// forward one, so a window may run back across several earlier joins. Version
// 11 (build 060: "what I really want is more lanes... what about a third
// lane then"): a third lane, C, written after B exactly as B is after A;
// nothing else changes. Version 12 (build 063: "you mentioned something
// about a 64 bar limit, so I can't make a variation span an entire song if it's
// longer than that?", "I'd like a song spanning variation to be an option"):
// only the ranges grow: a window up to 4096 steps (256 bars), a reach of up to
// 95 joins each way (every join a 96-entry song has), starts and offsets to
// match; the bytes are laid out as before.
#pragma once
#include <opxy/extension.hpp>
#include "engine.hpp"
#include "line.hpp"
#include "stagger.hpp"

namespace variations {
const u32 kCapacity = 8192;
const u32 kMaxVariations = 64;  // keeps the index at about 4 KB
const u32 kTracks = 8;
// Build 060 (version 11: "what I really want is more lanes... what about a third lane then"):
// three lanes, A, B and C. Version 10 and below have kLanesV10 of them: their bytes, and the
// version 1-4 intermediate upgrade() writes, hold two lanes a variation, and load with lane C empty.
const u32 kLanes = 3;
const u32 kLanesV10 = 2;
// Steps from the join either way: build 024 (version 5) lets a window lie
// wholly in the section before the join (to its start, 128 bars at most), where
// before it ended on the join at the earliest (offset >= -length).
// Build 063 (version 12: "I'd like a song spanning variation to be an option"): twice the longest window less a
// step, the ratio 2047 had to 1024 (2047 until version 12), so a window of the longest length may end on its join or
// lie wholly in the section before it. An i16 holds it (8191 of 32767).
const u32 kMaxLength = 4096;  // steps (1024 until version 12, 256 in version 8, 128 before; "a 64 bar limit")
const u32 kMaxOffset = 2 * kMaxLength - 1;
const u32 kNoScene = 0xff;  // a variation's scene either side of its join, not known (version 1 data)
const u32 kSceneCount = 99;
const u32 kVersion = 12;
const u32 kLaneRate = 0x30;  // lane flags bits 4-5 (version 7): log2 of the lane's rate, 0..3
const u32 kLaneMuted = 0x40;  // lane flags bit 6 (version 8, build 047): kept and edited, plays nothing
const u32 kReaching = 4;  // variation header byte 6 bit 2 (version 8): a reach list follows the header
const u32 kBackReaching = 8;  // variation header byte 6 bit 3 (version 10, build 058): a backward reach list follows the forward one
// Build 063 (version 12: "you mentioned something about a 64 bar limit, so I can't make a variation span an
// entire song if it's longer than that?"; "I'd like a song spanning variation to be an option"): a song of 2-bar
// scenes may have up to 96 entries (stock's most), so a window may cross 95 joins each way, and run to 256 bars.
const u32 kMaxReach = 95;  // later joins a window may cross after its own (31 until version 12, 15 until version 9)
// Steps from the window's join to a crossed join, forward or back (4095 until version 12; the backward list's too, build 058).
// The furthest forward join a window can cross is before its end, offset + length <= kMaxOffset + kMaxLength, so
// one step less than that, 12286; going back, a join is before the window's start, 8190 at the most, which this covers.
// It fits the u16 a record stores it in, and Draft's reach_start.
const u32 kMaxReachStart = kMaxOffset + kMaxLength - 1;
// What versions 9-11 allowed (build 063: a store of those versions is read within these, as before).
const u32 kMaxOffsetV11 = 2047, kMaxLengthV11 = 1024, kMaxReachV11 = 31, kMaxReachStartV11 = 4095;
const u32 kLaneNamed = 8;  // lane flags bit 3 (version 6): an engine control by name, the name after the curve
inline bool scene_ok(u32 s) { return s < kSceneCount || s == kNoScene; }

// A lane's target: a track word (opxy::extensions::track_targets), the
// sequenced-note line, or none (an empty lane).
const u32 kTargetNotes = 0x40;
const u32 kNoTarget = 0xff;
const u32 kVolume = 15, kCutoff = 16, kSend = 22;  // the targets of version 2's lines
inline bool target_ok(u32 t) {
    return t == kTargetNotes || t == kNoTarget || opxy::extensions::track_target(t) != nullptr;
}
// Gestures (gestures.hpp): 0 custom .. 7 dip, 8 fade in, 9 fade out (build
// 016, the same version 4 layout), 10 random, 11 wander, 12 cut in, 13 gate (stutter until build 046),
// 14 stutter (build 046, a note retrigger with no line; build 048: steady, its line flat at the top),
// 15 stutter in, 16 spring, 17 stutter out (build 048: the density shapes as variations)
// (build 040, no format change); kNoGesture an empty lane.
const u32 kGestureCount = 19;  // build 062: 18 hold ("flat line gesture"): a flat line at the bottom, any target, the volume to start
const u32 kFadeIn = 8, kFadeOut = 9, kHold = 18;  // kHold: build 062 ("flat line / silence curve"), not fixed to the volume, as the fades since 039
// fade in and fade out always move the volume: their target is fixed.
// Build 062: named for what it does. It picks natural_target's default only - build 039 freed the fades to
// take any target, and nothing has restricted them since, though the name said otherwise.
inline bool starts_on_volume(u32 g) { return g == kFadeIn || g == kFadeOut; }
const u32 kNoGesture = 0xff;
inline bool gesture_ok(u32 g) { return g < kGestureCount || g == kNoGesture; }
// Build 044: gate (gesture 13) stores one bar of its gate; how many of the
// window's steps a step of that bar spans, which playback repeats once a bar
// (window.hpp; 1 for windows up to a bar).
const u32 kGestureGate = 13, kGestureStutter = 14, kGestureStutterUp = 15, kGestureStutterSpring = 16, kGestureStutterDown = 17;
// The stutters (build 048): a note retrigger whose line is its density (retrigger.hpp).
inline bool is_stutter(u32 g) { return g >= kGestureStutter && g <= kGestureStutterDown; }
inline u32 stutter_scale(u32 length) { return length > 16 ? length / 16 : 1; }
// A lane's curve: the bend of its gesture's shape, -127..127 as a point's
// (line.hpp: above 0 a slow start, below 0 a fast start, 0 straight).
// Gestures.hpp applies it: rise and fall bend their ramp by it; swell and dip
// their way out by it and their way back by its mirror; build and break spread
// their entries and exits by it; drop and custom ignore it.
const int kSlowStart = 60, kFastStart = -60, kEaseOut = -40, kEaseIn = 40;
inline bool curve_ok(i32 c) { return c >= -127 && c <= 127; }
// Each gesture's own curve: a new lane's, and an older lane's as it loads
// (version 3 and before had none, and these are the shapes they made):
// rise a slow start, fall a fast one, swell and dip out fast and back slow,
// build and break evenly spread (straight); fade in as rise, fade out as fall.
inline i32 default_curve(u32 gesture) {
    return gesture == 4 || gesture == kFadeIn    ? kSlowStart
           : gesture == 5 || gesture == kFadeOut ? kFastStart
           : gesture == 6 || gesture == 7        ? kEaseOut
                                                 : 0;
}

// One lane as stored. Its line is `count` Points at byte `first` of the
// store's bytes (3 a point: step, value, curve, Point's layout).
struct Lane {
    unsigned char gesture, amount, target, selection;
    signed char curve;    // the gesture's bend (default_curve when made)
    bool default_tracks;  // the selection follows the gesture and position
    bool default_target;  // the target follows the gesture (volume or cutoff)
    bool edited;          // its gesture's points edited on the shape page: shown as custom (flags bit 2)
    bool muted;           // version 8 (flags bit 6): kept and edited, but plays nothing
    unsigned char rate;   // version 7 (flags bits 4-5): 1, 2, 4 or 8, how many times its line plays across the window
    unsigned short first;
    unsigned char count;  // 0: no points
    // Version 6: an engine control by name (flags bit 3), `name_length`
    // characters at byte `name_at`; 0: none (an engine target by slot).
    unsigned short name_at;
    unsigned char name_length;
};
struct Variation {
    unsigned char song, entry;  // 0-based
    short offset;               // steps from the join at the start of `entry`
    unsigned short length;      // steps, 8..4096 (1024 until version 12, 256 in version 8, 128 before)
    // Version 8: how many later joins the window crosses after its own (0: none),
    // and the byte offset of the first of the `reach` {scene, u16 start} records.
    unsigned char reach;
    unsigned short reach_at;
    // Build 058 (version 10): the same backward: how many earlier joins the window's start crosses
    // before its own (0: none), and the byte offset of the first of the `back` {scene, u16 start} records.
    unsigned char back;
    unsigned short back_at;
    bool unplaced;              // its scene pair is in no join of the song: kept, not played
    unsigned char scene_before, scene_after;  // the scenes either side of its join, or kNoScene
    Lane lanes[kLanes];
    unsigned short start, end;  // its bytes in the store: [start, end)
};

// A variation being edited (the page, tools/variations.py's generator): its
// header and its lanes' lines, as the store holds them once put.
const u32 kDraftPoints = 32;  // a line's most, as the format allows
struct DraftLane {
    u32 gesture, amount, target, selection;
    bool default_tracks, default_target;
    bool muted;  // version 8 (build 047)
    // The shape page edited its gesture's points (flags bit 2): the page shows
    // it as custom and keeps its points until the variation page changes its
    // gesture, amount or tracks, which regenerate them from the gesture.
    bool edited;
    u32 rate;  // version 7: 1, 2, 4 or 8 (the line plays that many times across the window)
    i32 curve;  // -127..127, the gesture's bend: its own (default_curve), or as saved by builds 015-019
    // Build 040: random and wander's dice, in memory only (never saved: the
    // stored points are the result); a lane loaded from the store has 0.
    u32 seed;
    // Version 6 (build 025): an engine target (word 0..7) moved by the
    // control's name on each track (engine.hpp), or empty: by its slot.
    char name[kNameCapacity];
    unsigned char count;  // the line's points: every track of the lane follows it
    Point points[kDraftPoints];
};
struct Draft {
    u32 song, entry;
    i32 offset;
    u32 length;
    // Version 8 (build 047): the later joins the window crosses, each the scene
    // of the entry after it and its distance in steps from the variation's own join.
    u32 reach;
    // Build 063 ("I'd like a song spanning variation to be an option"): a scene (0..98, kNoScene 0xff) and a
    // step count (to kMaxReachStart, 12286) are all they hold, so u8 and u16: at 95 records each way they were
    // 4 * 380 bytes as u32, twice in every Draft the page holds.
    u8 reach_scene[kMaxReach];
    u16 reach_start[kMaxReach];
    // Build 058 (version 10): the earlier joins the window's start crosses, each the scene of the
    // entry before it and its distance in steps forward from the join into that entry to the variation's own join.
    u32 back;
    u8 back_scene[kMaxReach];
    u16 back_start[kMaxReach];
    bool unplaced;
    u32 scene_before, scene_after;  // kNoScene: not known (no default here: C++11 aggregates, no static constructors)
    DraftLane lanes[kLanes];
};
// Sizes once encoded (notes/variations-format.md): a variation's 9 bytes, 7 a
// lane (its point count last), 3 a point; version 6: a named engine control's
// lane 1 + its name's length more. Version 4: 7 a lane, then 2 + 3n a
// track's line; version 3: 6 a lane; version 2: 14 a variation; version 1: 12.
const u32 kHeader = 9, kLaneHeader = 7, kLaneHeaderV3 = 6, kHeaderV2 = 14, kHeaderV1 = 12;
// A lane's name as stored: only on an engine target, at most kNameChars.
inline u32 name_length(const DraftLane& l) {
    if (l.target >= opxy::extensions::engine_words) return 0;
    u32 n = 0;
    while (n < kNameChars && l.name[n]) ++n;
    return n;
}
inline u32 encoded_size(const Draft& d) {
    u32 n = kHeader + kLanes * kLaneHeader + (d.reach ? 1 + 3 * d.reach : 0) + (d.back ? 1 + 3 * d.back : 0);
    for (const DraftLane& l : d.lanes) {
        n += 3 * l.count;
        if (const u32 k = name_length(l)) n += 1 + k;
    }
    return n;
}
// A name's characters: printable, lowercase as the SDK gives them.
inline bool name_char(u32 c) { return c >= 0x20 && c < 0x7f && !(c >= 'A' && c <= 'Z'); }
// A rate (1, 2, 4, 8) as its 2 bits of the lane's flags, log2; anything else 0.
inline u32 rate_bits(u32 rate) { return rate == 2 ? 1 : rate == 4 ? 2 : rate == 8 ? 3 : 0; }
inline bool rate_ok(u32 rate) { return rate == 1 || rate == 2 || rate == 4 || rate == 8; }
inline void clear_points(DraftLane& l) { l.count = 0; }
inline void clear_lane(DraftLane& l) {
    l.gesture = kNoGesture, l.amount = 128, l.target = kNoTarget, l.selection = 0;
    l.default_tracks = false, l.default_target = true, l.edited = false;
    l.muted = false;
    l.rate = 1;
    l.curve = 0;
    l.seed = 0;
    l.name[0] = 0;
    clear_points(l);
}

// Version 2's lines as version 3 targets and values (a line's value 0..255 to
// a lane's, 128 as set): level 255 as set -> 128; send 0 as set -> 128; notes
// and filter already centred.
namespace legacy {
enum Line : u32 { level = 0, notes = 1, send = 2, filter = 3 };
const u32 kLines = 4;
const u32 kTargets[kLines] = {kVolume, kTargetNotes, kSend, kCutoff};
inline u32 value(u32 line, u32 v) {
    if (line == level) return (v * 128 + 127) / 255;
    if (line == send) return 128 + (v * 127 + 127) / 255;
    return v;
}
// Version 2 gestures: 0 custom, 1 build, 2 break, 3 drop, 4 swell (the send
// rising), 5 filter rise, 6 filter fall, 7 clear -> version 3's (gestures.hpp).
inline u32 gesture(u32 g) {
    const u32 map[8] = {0, 1, 2, 3, 4, 4, 5, kNoGesture};
    return g < 8 ? map[g] : kNoGesture;
}
// The line a version 2 gesture moved: lane A takes it.
inline u32 natural_line(u32 g) { return g == 4 ? send : g == 5 || g == 6 ? filter : level; }
}
// The target a gesture moves while the lane's target follows it (fade in and
// fade out: always the volume).
inline u32 natural_target(u32 gesture) {
    if (is_stutter(gesture)) return kTargetNotes;  // build 046: no line; shown as "notes"
    if (starts_on_volume(gesture) || gesture == 12 || gesture == 13 || gesture == kHold) return kVolume;  // fades, cut in, gate, hold (build 062)
    return gesture >= 4 && gesture < kGestureCount ? kCutoff : gesture == kNoGesture ? kNoTarget : kVolume;
}

class Store {
public:
    // Replaces what is held with these bytes; false (holding none) if they are
    // not valid variations. Version 1-4 bytes are rewritten as version 11
    // (version 5-10 bytes get the empty lane C of version 11 written in);
    // should that not fit (kCapacity bytes or kMaxVariations: only a version 1
    // or 2 store within about 450 bytes of full, or a full one whose build and
    // break lanes had no points), they are kept as they are, saved back
    // unchanged, neither played nor edited (kept()).
    bool load(const u8* data, u32 bytes) {
        ++generation_;
        kept_ = false;
        count_ = 0;
        if (bytes >= 4 && data[0] == 'V' && data[1] == 'R' && data[2] >= 1 && data[2] < 5) {
            if (bytes > kCapacity) return false;
            bool valid = true, fits;
            if (data[2] <= 2) {
                valid = old_valid(data, bytes);
                fits = valid && upgrade(data, bytes) && to_v6(kLaneHeader, &valid);
            } else {
                for (u32 i = 0; i < bytes; ++i) bytes_[i] = data[i];
                size_ = bytes;
                fits = to_v6(data[2] == 3 ? kLaneHeaderV3 : kLaneHeader, &valid);
            }
            if (!valid) {
                count_ = 0, size_ = 0;
                return false;
            }
            if (!fits) {
                for (u32 i = 0; i < bytes; ++i) bytes_[i] = data[i];
                size_ = bytes;
                count_ = 0;
                kept_ = true;
            }
            return true;
        }
        return parse_all(data, bytes);
    }
    // (Version 5 bytes: parse_all reads them as version 6 and renumbers them.)
    void clear() { count_ = 0, size_ = 0, kept_ = false, ++generation_; }
    // Older bytes too full to rewrite: held only to save them back.
    bool kept() const { return kept_; }

    // Editing (the page): variation i replaced by d, or d added after the
    // others when i is count(). False, changing nothing, when it would not fit
    // (kCapacity bytes, kMaxVariations) or d is not a valid variation.
    bool put(u32 i, const Draft& d) {
        if (kept_ || i > count_ || (i == count_ && count_ >= kMaxVariations) || !valid(d)) return false;
        if (!count_) header();
        const u32 start = i < count_ ? list_[i].start : size_, end = i < count_ ? list_[i].end : size_;
        const u32 need = encoded_size(d), size = size_ - (end - start) + need;
        if (size > kCapacity) return false;
        move(start + need, end, size_ - end);
        write(start, d);
        size_ = size;
        bytes_[3] = static_cast<u8>(i == count_ ? count_ + 1 : count_);
        return reindex();
    }
    // Variation i removed; false if there is none.
    bool remove(u32 i) {
        if (i >= count_) return false;
        const u32 start = list_[i].start, end = list_[i].end;
        move(start, end, size_ - end);
        size_ -= end - start;
        bytes_[3] = static_cast<u8>(count_ - 1);
        return reindex();
    }
    // Variation i pinned to the join into `entry` with these scenes either
    // side, or marked unplaced; in place (the same size).
    bool pin(u32 i, u32 entry, u32 before, u32 after, bool unplaced) {
        if (i >= count_ || entry >= 96 || !scene_ok(before) || !scene_ok(after)) return false;
        Variation& v = list_[i];
        const u32 h = v.start;
        bytes_[h + 1] = static_cast<u8>(entry);
        bytes_[h + 6] = static_cast<u8>((bytes_[h + 6] & ~2u) | (unplaced ? 2u : 0u));
        bytes_[h + 7] = static_cast<u8>(before), bytes_[h + 8] = static_cast<u8>(after);
        v.entry = static_cast<unsigned char>(entry);
        v.unplaced = unplaced;
        v.scene_before = static_cast<unsigned char>(before), v.scene_after = static_cast<unsigned char>(after);
        return true;
    }
    // The bytes' version: 8, or 1-4 for older bytes kept as they are.
    u32 version() const { return kept_ ? bytes_[2] : kVersion; }

    // Variation i as a draft (its points included).
    void draft(u32 i, Draft& d) const {
        const Variation& v = list_[i];
        d.song = v.song, d.entry = v.entry, d.offset = v.offset, d.length = v.length;
        d.reach = v.reach;
        for (u32 j = 0; j < kMaxReach; ++j) d.reach_scene[j] = kNoScene, d.reach_start[j] = 0;
        for (u32 j = 1; j <= v.reach; ++j) {
            u32 scene, start;  // build 063: Draft's are u8 and u16
            reach(v, j, &scene, &start);
            d.reach_scene[j - 1] = static_cast<u8>(scene), d.reach_start[j - 1] = static_cast<unsigned short>(start);
        }
        d.back = v.back;  // build 058
        for (u32 j = 0; j < kMaxReach; ++j) d.back_scene[j] = kNoScene, d.back_start[j] = 0;
        for (u32 j = 1; j <= v.back; ++j) {
            u32 scene, start;
            back(v, j, &scene, &start);
            d.back_scene[j - 1] = static_cast<u8>(scene), d.back_start[j - 1] = static_cast<unsigned short>(start);
        }
        d.unplaced = v.unplaced;
        d.scene_before = v.scene_before, d.scene_after = v.scene_after;
        for (u32 n = 0; n < kLanes; ++n) {
            const Lane& l = v.lanes[n];
            DraftLane& o = d.lanes[n];
            o.seed = 0;  // build 040: the dice are not saved
            o.gesture = l.gesture, o.amount = l.amount, o.target = l.target, o.selection = l.selection;
            o.default_tracks = l.default_tracks, o.default_target = l.default_target;
            o.edited = l.edited;
            o.muted = l.muted;
            o.rate = l.rate;
            o.curve = l.curve;
            for (u32 k = 0; k < l.name_length; ++k) o.name[k] = static_cast<char>(bytes_[l.name_at + k]);
            o.name[l.name_length] = 0;
            u32 count;
            const Point* p = points(v, n, &count);
            o.count = static_cast<unsigned char>(count);
            for (u32 k = 0; k < count; ++k) o.points[k] = p[k];
        }
    }
    // What put() checks: the format's ranges, points in step order within the window.
    static bool valid(const Draft& d) {
        if (d.song >= 14 || d.entry >= 96 || d.length < 8 || d.length > kMaxLength) return false;
        if (d.offset < -static_cast<i32>(kMaxOffset) || d.offset > static_cast<i32>(kMaxOffset)) return false;
        if (!scene_ok(d.scene_before) || !scene_ok(d.scene_after)) return false;
        // Build 047: the reach list: scenes known or not, starts strictly increasing from 1
        // to kMaxReachStart (1023 until build 049), and the window really crossing the first of them.
        if (d.reach > kMaxReach) return false;
        for (u32 j = 0; j < d.reach; ++j)
            if (!scene_ok(d.reach_scene[j]) || d.reach_start[j] < 1 || d.reach_start[j] > kMaxReachStart || (j && d.reach_start[j] <= d.reach_start[j - 1]))
                return false;
        if (d.reach && d.offset + static_cast<i32>(d.length) <= static_cast<i32>(d.reach_start[0])) return false;
        // Build 058: the backward list, the same: scenes known or not, starts strictly rising within 1..kMaxReachStart,
        // and the window really reaching the last of its entries (build 064: starting at or before its start: offset <= -start_j,
        // so the last one's decides; 058: strictly before).
        if (d.back > kMaxReach) return false;
        for (u32 j = 0; j < d.back; ++j)
            if (!scene_ok(d.back_scene[j]) || d.back_start[j] < 1 || d.back_start[j] > kMaxReachStart || (j && d.back_start[j] <= d.back_start[j - 1]))
                return false;
        if (d.back && d.offset + static_cast<i32>(d.back_start[d.back - 1]) > 0) return false;
        for (const DraftLane& l : d.lanes) {
            if (!gesture_ok(l.gesture) || l.amount > 255 || !target_ok(l.target) || l.selection >> kTracks || !curve_ok(l.curve))
                return false;
            if (l.count > kDraftPoints || !rate_ok(l.rate)) return false;
            for (u32 k = 0; k < name_length(l); ++k)
                if (!name_char(static_cast<u8>(l.name[k]))) return false;
            for (u32 k = 0; k < l.count; ++k) {
                const Point& p = l.points[k];
                if (p.step >= line_length(d.length) || (k && p.step <= l.points[k - 1].step) || p.curve == -128) return false;
            }
        }
        return true;
    }

    // The bytes as held (what save gives back): none when nothing is held.
    const u8* bytes() const { return bytes_; }
    u32 size() const { return count_ || kept_ ? size_ : 0; }
    // Bumped by every load: an index taken before one no longer names the same variation.
    u32 generation() const { return generation_; }

    u32 count() const { return count_; }
    const Variation& operator[](u32 i) const { return list_[i]; }
    // A lane's engine control name (version 6) into `out` (kNameCapacity),
    // empty for none.
    void name(const Variation& v, u32 lane, char* out) const {
        const Lane& l = v.lanes[lane];
        for (u32 k = 0; k < l.name_length; ++k) out[k] = static_cast<char>(bytes_[l.name_at + k]);
        out[l.name_length] = 0;
    }
    // Build 047: the scene and start (steps from the variation's own join) of the
    // join j crosses, 1 <= j <= v.reach.
    void reach(const Variation& v, u32 j, u32* scene, u32* start) const {
        const u32 at = v.reach_at + 3 * (j - 1);
        *scene = bytes_[at], *start = u16(at + 1);
    }
    // Build 058: the scene and start (steps forward from the join into entry - j to the variation's
    // own join) of the join j crosses backward, 1 <= j <= v.back.
    void back(const Variation& v, u32 j, u32* scene, u32* start) const {
        const u32 at = v.back_at + 3 * (j - 1);
        *scene = bytes_[at], *start = u16(at + 1);
    }
    // A lane's line, or null when it has none (count 0).
    const Point* points(const Variation& v, u32 lane, u32* count) const {
        *count = v.lanes[lane].count;
        return *count ? reinterpret_cast<const Point*>(bytes_ + v.lanes[lane].first) : nullptr;
    }

private:
    bool parse_all(const u8* data, u32 bytes) {
        count_ = 0;
        if (!bytes) return true;  // a project with none
        if (bytes < 4 || bytes > kCapacity || data[0] != 'V' || data[1] != 'R' || data[2] < 5 || data[2] > kVersion) return false;
        if (data != bytes_)
            for (u32 i = 0; i < bytes; ++i) bytes_[i] = data[i];
        size_ = bytes;
        const u32 count = data[3];
        if (count > kMaxVariations) return false;
        u32 at = 4;
        const bool named = data[2] >= 6;  // version 5 had no names: its flags bit 3 means nothing
        const bool rated = data[2] >= 7;  // version 6 had no rates: its flags bits 4-5 mean nothing
        const u32 version = data[2];
        const bool v8 = version >= 8;  // version 5-7 had no muted lanes or reach lists: those bits mean nothing
        // Build 060: version 10 and below hold two lanes a variation, 11 three.
        const u32 lanes = version >= 11 ? kLanes : kLanesV10;
        for (u32 n = 0; n < count; ++n)
            if (!parse(list_[n], &at, named, rated, v8, version >= 9, version >= 10, version >= 12, lanes)) return false;
        if (at != bytes) return false;
        if (lanes < kLanes) {
            // Version 5-10: lane C is written in after each variation's last lane (every variation ends
            // with its lanes), working from the last so no byte is moved over one not yet moved. Too
            // full to take them: kept as they are, saved back unchanged and neither played nor edited.
            const u32 extra = (kLanes - lanes) * kLaneHeader;
            if (bytes + extra * count > kCapacity) {
                if (data != bytes_)
                    for (u32 i = 0; i < bytes; ++i) bytes_[i] = data[i];
                size_ = bytes, count_ = 0, kept_ = true;
                return true;
            }
            for (u32 i = count; i-- > 0;) {
                const Variation& v = list_[i];
                move(v.start + extra * i, v.start, v.end - v.start);
                u32 at2 = v.end + extra * i;
                for (u32 k = 0; k < kLanes - lanes; ++k) write_empty_lane(at2);
            }
            size_ = bytes + extra * count;
            bytes_[2] = static_cast<u8>(kVersion);
            return parse_all(bytes_, size_);
        }
        // Build 063: version 11 bytes are version 12's within the smaller ranges; nothing is written in, so only the
        // number changes (the bytes saved back are version 12: a build before 063 could not read them, as the others).
        bytes_[2] = static_cast<u8>(kVersion);
        count_ = count;
        return true;
    }
    // An empty lane's seven bytes at `at` (clear_lane's: no gesture, 128, no target, no tracks, the
    // target following the gesture, no curve, no points), `at` moved past them.
    void write_empty_lane(u32& at) {
        const u8 empty[kLaneHeader] = {static_cast<u8>(kNoGesture), 128, static_cast<u8>(kNoTarget), 0, 2, 0, 0};
        for (u32 k = 0; k < kLaneHeader; ++k) bytes_[at++] = empty[k];
    }
    void header() {
        bytes_[0] = 'V', bytes_[1] = 'R', bytes_[2] = static_cast<u8>(kVersion), bytes_[3] = 0;
        size_ = 4;
    }
    // d's bytes at `at` (room made for encoded_size(d)).
    void write(u32 at, const Draft& d) {
        bytes_[at++] = static_cast<u8>(d.song);
        bytes_[at++] = static_cast<u8>(d.entry);
        const u32 offset = static_cast<u32>(d.offset) & 0xffff;
        bytes_[at++] = static_cast<u8>(offset), bytes_[at++] = static_cast<u8>(offset >> 8);
        bytes_[at++] = static_cast<u8>(d.length), bytes_[at++] = static_cast<u8>(d.length >> 8);
        bytes_[at++] = static_cast<u8>((d.unplaced ? 2 : 0) | (d.reach ? kReaching : 0) | (d.back ? kBackReaching : 0));
        bytes_[at++] = static_cast<u8>(d.scene_before);
        bytes_[at++] = static_cast<u8>(d.scene_after);
        if (d.reach) {
            bytes_[at++] = static_cast<u8>(d.reach);
            for (u32 j = 0; j < d.reach; ++j) {
                bytes_[at++] = static_cast<u8>(d.reach_scene[j]);
                bytes_[at++] = static_cast<u8>(d.reach_start[j]), bytes_[at++] = static_cast<u8>(d.reach_start[j] >> 8);
            }
        }
        if (d.back) {  // build 058: the backward list, after the forward one
            bytes_[at++] = static_cast<u8>(d.back);
            for (u32 j = 0; j < d.back; ++j) {
                bytes_[at++] = static_cast<u8>(d.back_scene[j]);
                bytes_[at++] = static_cast<u8>(d.back_start[j]), bytes_[at++] = static_cast<u8>(d.back_start[j] >> 8);
            }
        }
        for (const DraftLane& l : d.lanes) {
            bytes_[at++] = static_cast<u8>(l.gesture);
            bytes_[at++] = static_cast<u8>(l.amount);
            bytes_[at++] = static_cast<u8>(l.target);
            bytes_[at++] = static_cast<u8>(l.selection);
            const u32 name = name_length(l);
            bytes_[at++] = static_cast<u8>((l.default_tracks ? 1 : 0) | (l.default_target ? 2 : 0) |
                                           (l.edited && l.gesture != kNoGesture ? 4 : 0) | (name ? kLaneNamed : 0) | (rate_bits(l.rate) << 4) |
                                           (l.muted ? kLaneMuted : 0));
            bytes_[at++] = static_cast<u8>(l.curve);
            if (name) {
                bytes_[at++] = static_cast<u8>(name);
                for (u32 k = 0; k < name; ++k) bytes_[at++] = static_cast<u8>(l.name[k]);
            }
            bytes_[at++] = l.count;
            for (u32 k = 0; k < l.count; ++k) {
                const Point& p = l.points[k];
                bytes_[at++] = p.step, bytes_[at++] = p.value;
                bytes_[at++] = static_cast<u8>(p.curve);
            }
        }
    }
    // bytes_[from, from + n) to `to`, either way (no library calls on the device).
    void move(u32 to, u32 from, u32 n) {
        if (to < from)
            for (u32 k = 0; k < n; ++k) bytes_[to + k] = bytes_[from + k];
        else if (to > from)
            for (u32 k = n; k-- > 0;) bytes_[to + k] = bytes_[from + k];
    }
    // The index again after an edit, from the bytes as they now are.
    bool reindex() {
        const bool ok = parse_all(bytes_, size_);
        if (!ok) clear();  // never: put() and remove() write valid bytes
        return ok;
    }
    static bool take(u32 size, u32* at, u32 n) { return *at + n <= size && (*at += n, true); }
    u32 u16(u32 at) const { return bytes_[at] | bytes_[at + 1] << 8; }

    bool parse(Variation& v, u32* at, bool named, bool rated, bool v8, bool v9, bool v10, bool v12, u32 lanes) {
        const u32 h = *at;
        if (!take(size_, at, kHeader)) return false;
        const u32 song = bytes_[h], entry = bytes_[h + 1], length = u16(h + 4);
        const u32 before = bytes_[h + 7], after = bytes_[h + 8];
        const i32 offset = static_cast<short>(u16(h + 2));
        // Build 063 (version 12): the ranges of version 11 and below as they were (kMax...V11), the new ones from version 12.
        const u32 max_length = v12 ? kMaxLength : v9 ? kMaxLengthV11 : v8 ? 256 : 128, max_offset = v12 ? kMaxOffset : kMaxOffsetV11;
        if (song >= 14 || entry >= 96 || length < 8 || length > max_length || !scene_ok(before) || !scene_ok(after)) return false;
        if (offset < -static_cast<i32>(max_offset) || offset > static_cast<i32>(max_offset)) return false;
        if (!v8) bytes_[h + 6] = static_cast<u8>(bytes_[h + 6] & ~kReaching);
        if (!v10) bytes_[h + 6] = static_cast<u8>(bytes_[h + 6] & ~kBackReaching);  // build 058: no backward list before version 10
        v.reach = 0, v.reach_at = 0;
        if (bytes_[h + 6] & kReaching) {
            // Version 8: a reach list, 1..15 records {scene, u16 start}, starts strictly rising to 1023;
            // version 9 (build 049): 1..31 records, to 4095; version 12 (build 063): 1..95 records, to kMaxReachStart.
            const u32 k = *at;
            if (!take(size_, at, 1)) return false;
            const u32 reach = bytes_[k];
            if (reach < 1 || reach > (v12 ? kMaxReach : v9 ? kMaxReachV11 : 15) || !take(size_, at, 3 * reach)) return false;
            u32 last = 0;
            for (u32 j = 0; j < reach; ++j) {
                const u32 scene = bytes_[k + 1 + 3 * j], start = u16(k + 2 + 3 * j);
                if (!scene_ok(scene) || start <= last || start > (v12 ? kMaxReachStart : v9 ? kMaxReachStartV11 : 1023)) return false;
                last = start;
            }
            v.reach = static_cast<unsigned char>(reach), v.reach_at = static_cast<unsigned short>(k + 1);
        }
        v.back = 0, v.back_at = 0;
        if (bytes_[h + 6] & kBackReaching) {
            // Version 10 (build 058): a backward list, 1..31 records {scene, u16 start}, starts strictly rising to 4095;
            // version 12 (build 063): 1..95 records, to kMaxReachStart.
            const u32 k = *at;
            if (!take(size_, at, 1)) return false;
            const u32 back = bytes_[k];
            if (back < 1 || back > (v12 ? kMaxReach : kMaxReachV11) || !take(size_, at, 3 * back)) return false;
            u32 last = 0;
            for (u32 j = 0; j < back; ++j) {
                const u32 scene = bytes_[k + 1 + 3 * j], start = u16(k + 2 + 3 * j);
                if (!scene_ok(scene) || start <= last || start > (v12 ? kMaxReachStart : kMaxReachStartV11)) return false;
                last = start;
            }
            v.back = static_cast<unsigned char>(back), v.back_at = static_cast<unsigned short>(k + 1);
        }
        v.song = static_cast<unsigned char>(song);
        v.entry = static_cast<unsigned char>(entry);
        v.offset = static_cast<short>(offset);
        v.length = static_cast<unsigned short>(length);
        v.unplaced = bytes_[h + 6] & 2;
        v.scene_before = static_cast<unsigned char>(before);
        v.scene_after = static_cast<unsigned char>(after);
        v.start = static_cast<unsigned short>(h);
        for (u32 ln = 0; ln < lanes; ++ln) {
            Lane& l = v.lanes[ln];
            const u32 lh = *at;
            if (!take(size_, at, kLaneHeader - 1)) return false;
            if (!gesture_ok(bytes_[lh]) || !target_ok(bytes_[lh + 2])) return false;
            l.gesture = bytes_[lh], l.amount = bytes_[lh + 1], l.target = bytes_[lh + 2], l.selection = bytes_[lh + 3];
            if (!named) bytes_[lh + 4] = static_cast<u8>(bytes_[lh + 4] & ~kLaneNamed);
            if (!rated) bytes_[lh + 4] = static_cast<u8>(bytes_[lh + 4] & ~kLaneRate);
            if (!v8) bytes_[lh + 4] = static_cast<u8>(bytes_[lh + 4] & ~kLaneMuted);
            l.muted = bytes_[lh + 4] & kLaneMuted;
            l.default_tracks = bytes_[lh + 4] & 1;
            l.default_target = bytes_[lh + 4] & 2;
            l.edited = bytes_[lh + 4] & 4;
            l.rate = static_cast<unsigned char>(1 << (bytes_[lh + 4] >> 4 & 3));
            const i32 curve = static_cast<signed char>(bytes_[lh + 5]);
            if (!curve_ok(curve)) return false;
            l.curve = static_cast<signed char>(curve);
            l.name_at = 0, l.name_length = 0;
            if (bytes_[lh + 4] & kLaneNamed) {
                // Version 6: an engine control's name, 1..15 characters.
                const u32 k = *at;
                if (!take(size_, at, 1) || l.target >= opxy::extensions::engine_words) return false;
                const u32 length = bytes_[k];
                if (length < 1 || length > kNameChars || !take(size_, at, length)) return false;
                for (u32 c = 0; c < length; ++c)
                    if (!name_char(bytes_[k + 1 + c])) return false;
                l.name_at = static_cast<unsigned short>(k + 1), l.name_length = static_cast<unsigned char>(length);
            }
            const u32 count_at = *at;
            if (!take(size_, at, 1)) return false;
            const u32 n = bytes_[count_at], first = *at;
            if (n > kDraftPoints || !take(size_, at, 3 * n) || !points_ok(bytes_ + first, n, line_length(length))) return false;
            l.first = static_cast<unsigned short>(first);
            l.count = static_cast<unsigned char>(n);
        }
        v.end = static_cast<unsigned short>(*at);
        return true;
    }
    static bool points_ok(const u8* p, u32 n, u32 length) {
        u32 last = 0;
        for (u32 i = 0; i < n; ++i) {
            const u32 step = p[3 * i];
            if (step >= length || (i && step <= last) || p[3 * i + 2] == 0x80) return false;
            last = step;
        }
        return true;
    }

    // Version 1 and 2 (notes/variations-format.md): each variation's header,
    // then runs of {track | line << 4, n, n points}. Walks them, calling
    // `each(h, header, runs_at, end)` per variation; false on any fault.
    template <class Each>
    static bool walk_old(const u8* data, u32 bytes, Each each) {
        if (bytes < 4 || bytes > kCapacity || data[3] > kMaxVariations) return false;
        const u32 header = data[2] == 1 ? kHeaderV1 : kHeaderV2;
        u32 at = 4;
        for (u32 n = 0; n < data[3]; ++n) {
            const u32 h = at;
            if (!take(bytes, &at, header)) return false;
            const u32 length = data[h + 4] | data[h + 5] << 8, selection = data[h + 8] | data[h + 9] << 8;
            const i32 offset = static_cast<short>(data[h + 2] | data[h + 3] << 8);
            if (data[h] >= 14 || data[h + 1] >= 96 || length < 8 || length > 128 || data[h + 6] > 7 || selection >> kTracks) return false;
            if (offset < -static_cast<i32>(length) || offset > static_cast<i32>(kMaxOffsetV11)) return false;  // build 063: version 1-2's range as it was
            if (header == kHeaderV2 && (!scene_ok(data[h + 11]) || !scene_ok(data[h + 12]))) return false;
            u32 seen[kTracks] = {};
            for (u32 r = 0; r < data[h + header - 1]; ++r) {
                const u32 p = at;
                if (!take(bytes, &at, 2)) return false;
                const u32 track = data[p] & 15, line = data[p] >> 4, count = data[p + 1];
                if (track >= kTracks || line >= legacy::kLines || count < 1 || count > kDraftPoints || seen[track] >> line & 1) return false;
                seen[track] |= 1u << line;
                if (!take(bytes, &at, 3 * count) || !points_ok(data + p + 2, count, length)) return false;
            }
            each(h, header, h + header, at);
        }
        return at == bytes;
    }
    static bool old_valid(const u8* data, u32 bytes) {
        return walk_old(data, bytes, [](u32, u32, u32, u32) {});
    }
    // Version 1 or 2 bytes as version 4 in bytes_ (to_v6 then takes them on): lane A the line the
    // gesture moved (or, for custom, the first line with points), with its
    // gesture, amount and tracks; lane B the next line with points (build and
    // break thinned or filled the other tracks' notes), custom, on the tracks
    // with points. A third and fourth line (custom data only) become a second
    // variation on the same window, custom, so nothing changes in the sound.
    // False if the result would not fit (kCapacity bytes, kMaxVariations).
    bool upgrade(const u8* data, u32 bytes) {
        if (!old_valid(data, bytes)) return false;
        u32 out = 4, count = 0;
        bool fits = true;
        walk_old(data, bytes, [&](u32 h, u32 header, u32 runs_at, u32 end) {
            if (!fits) return;
            // Which lines have points, and where each track's run starts.
            u32 used = 0, run[kTracks][legacy::kLines] = {};
            for (u32 at = runs_at; at < end;) {
                const u32 track = data[at] & 15, line = data[at] >> 4, n = data[at + 1];
                used |= 1u << line;
                run[track][line] = at;
                at += 2 + 3 * n;
            }
            const u32 g = data[h + 6];
            u32 first = g == 0 ? legacy::kLines : legacy::natural_line(g);
            for (u32 l = 0; first == legacy::kLines && l < legacy::kLines; ++l)
                if (used >> l & 1) first = l;
            if (first == legacy::kLines) first = legacy::level;
            u32 order[legacy::kLines + 1] = {first, legacy::kLines, legacy::kLines, legacy::kLines, legacy::kLines}, lines = 1;
            for (u32 l = 0; l < legacy::kLines; ++l)
                if (l != first && used >> l & 1) order[lines++] = l;
            // Build 060: this writes version 4 as an intermediate (to_v6 takes it on), whose
            // variations hold two lanes, so the lines go two at a time, a pair a variation: kLanesV10,
            // not kLanes (which is 3 now: the third lane of the pair would be line 0, `level`, again,
            // and the version 4 bytes would be three lanes long for read_old's two). Lane C is made
            // empty by read_old, as for any version 10 or below.
            for (u32 c = 0; fits && c < lines; c += kLanesV10) {
                const u32 pair[kLanesV10] = {order[c], order[c + 1]};
                u32 need = kHeader + kLanesV10 * kLaneHeader;
                for (u32 line : pair)
                    for (u32 t = 0; line < legacy::kLines && t < kTracks; ++t)
                        if (run[t][line]) need += 2 + 3 * data[run[t][line] + 1];
                if (out + need > kCapacity || count >= kMaxVariations) {
                    fits = false;
                    return;
                }
                u32 at = out;
                bytes_[at++] = data[h], bytes_[at++] = data[h + 1];
                bytes_[at++] = data[h + 2], bytes_[at++] = data[h + 3];
                bytes_[at++] = data[h + 4], bytes_[at++] = data[h + 5];
                bytes_[at++] = static_cast<u8>(header == kHeaderV2 ? data[h + 10] & 2 : 0);
                bytes_[at++] = static_cast<u8>(header == kHeaderV2 ? data[h + 11] : kNoScene);
                bytes_[at++] = static_cast<u8>(header == kHeaderV2 ? data[h + 12] : kNoScene);
                for (u32 n = 0; n < kLanesV10; ++n) {
                    const u32 line = pair[n];
                    const bool primary = c == 0 && n == 0;  // the variation's own gesture, amount and tracks
                    u32 selection = 0;
                    for (u32 t = 0; line < legacy::kLines && t < kTracks; ++t)
                        if (run[t][line]) selection |= 1u << t;
                    if (primary) selection = data[h + 8];
                    const u32 gesture = primary ? legacy::gesture(g) : line < legacy::kLines ? 0 : kNoGesture;
                    const u32 target = gesture == kNoGesture ? kNoTarget : legacy::kTargets[line];
                    const bool natural = primary && gesture != 0 && gesture != kNoGesture && target == natural_target(gesture);
                    bytes_[at++] = static_cast<u8>(gesture);
                    bytes_[at++] = static_cast<u8>(primary ? data[h + 7] : 0);
                    bytes_[at++] = static_cast<u8>(target);
                    bytes_[at++] = static_cast<u8>(selection);
                    bytes_[at++] = static_cast<u8>((primary && (data[h + 10] & 1) ? 1 : 0) | (natural || line >= legacy::kLines ? 2 : 0));
                    bytes_[at++] = static_cast<u8>(default_curve(gesture));
                    const u32 runs_at2 = at++;
                    u32 runs = 0;
                    for (u32 t = 0; line < legacy::kLines && t < kTracks; ++t) {
                        const u32 p = run[t][line];
                        if (!p) continue;
                        ++runs;
                        const u32 points = data[p + 1];
                        bytes_[at++] = static_cast<u8>(t), bytes_[at++] = static_cast<u8>(points);
                        for (u32 k = 0; k < points; ++k) {
                            const u8* q = data + p + 2 + 3 * k;
                            bytes_[at++] = q[0];
                            bytes_[at++] = static_cast<u8>(legacy::value(line, q[1]));
                            bytes_[at++] = q[2];
                        }
                    }
                    bytes_[runs_at2] = static_cast<u8>(runs);
                }
                out = at;
                ++count;
            }
        });
        if (!fits) return false;
        bytes_[0] = 'V', bytes_[1] = 'R', bytes_[2] = 4, bytes_[3] = static_cast<u8>(count);
        size_ = out;
        return true;
    }

    // A version 3 (lane_header 6) or 4 (7) variation at `*at` of `data` (its
    // first `bytes`) as a version 6 draft (no names): false on any fault. Each lane's one
    // line: its leading track's (stagger.hpp: build the lowest of its tracks,
    // break the highest, the rest the lowest), or with no line there the
    // lowest track's that has one (a custom lane: the lowest track's with
    // points, its tracks those with points); build and break (unless edited) made again
    // from the gesture, so their stagger puts every track's entry or exit
    // where version 4's jumps were. Version 3's lanes take their gesture's own
    // curve (it had none).
    static bool read_old(const u8* data, u32 bytes, u32* at, u32 lane_header, Draft& d) {
        const u32 h = *at;
        if (!take(bytes, at, kHeader)) return false;
        const u32 length = data[h + 4] | data[h + 5] << 8;
        const i32 offset = static_cast<short>(data[h + 2] | data[h + 3] << 8);
        d.song = data[h], d.entry = data[h + 1], d.length = length, d.offset = offset;
        d.reach = 0, d.back = 0;
        d.unplaced = data[h + 6] & 2;
        d.scene_before = data[h + 7], d.scene_after = data[h + 8];
        if (d.song >= 14 || d.entry >= 96 || length < 8 || length > 128 || !scene_ok(d.scene_before) || !scene_ok(d.scene_after))
            return false;
        if (offset < -static_cast<i32>(length) || offset > static_cast<i32>(kMaxOffsetV11)) return false;  // build 063: version 3-4's range as it was
        for (u32 ln = 0; ln < kLanes; ++ln) {
            DraftLane& l = d.lanes[ln];
            // Build 060: version 3 and 4 hold two lanes; lane C (and any beyond) is empty.
            if (ln >= kLanesV10) {
                clear_lane(l);
                continue;
            }
            const u32 lh = *at;
            if (!take(bytes, at, lane_header)) return false;
            if (!gesture_ok(data[lh]) || !target_ok(data[lh + 2])) return false;
            l.gesture = data[lh], l.amount = data[lh + 1], l.target = data[lh + 2], l.selection = data[lh + 3];
            l.default_tracks = data[lh + 4] & 1;
            l.default_target = data[lh + 4] & 2;
            l.edited = lane_header == kLaneHeader && (data[lh + 4] & 4) && l.gesture != kNoGesture;
            l.rate = 1;
            l.muted = false;
            l.curve = lane_header == kLaneHeader ? static_cast<signed char>(data[lh + 5]) : default_curve(l.gesture);
            l.name[0] = 0;
            if (!curve_ok(l.curve)) return false;
            const i32 lead = leading_track(l.gesture, l.selection);
            u32 seen = 0, line_at = 0, line_n = 0, fallback_at = 0, fallback_n = 0, fallback_track = kTracks;
            for (u32 r = 0, runs = data[lh + lane_header - 1]; r < runs; ++r) {
                const u32 p = *at;
                if (!take(bytes, at, 2)) return false;
                const u32 track = data[p], n = data[p + 1];
                if (track >= kTracks || n < 1 || n > kDraftPoints || (seen >> track & 1)) return false;
                seen |= 1u << track;
                const u32 first = *at;
                if (!take(bytes, at, 3 * n) || !points_ok(data + first, n, length)) return false;
                if (static_cast<i32>(track) == lead) line_at = first, line_n = n;
                if (track < fallback_track) fallback_track = track, fallback_at = first, fallback_n = n;
            }
            if (!line_n) line_at = fallback_at, line_n = fallback_n;
            // A custom lane played the tracks with points (version 4 drove
            // those, whatever its selection): they are its tracks now, and
            // its line the lowest one's.
            if (l.gesture == 0 && seen) l.selection = seen, line_at = fallback_at, line_n = fallback_n;
            l.count = static_cast<unsigned char>(line_n);
            for (u32 k = 0; k < line_n; ++k) {
                const u8* q = data + line_at + 3 * k;
                l.points[k] = Point{q[0], q[1], static_cast<signed char>(q[2])};
            }
            if (staggers(l.gesture) && !l.edited && l.selection)
                l.count = static_cast<unsigned char>(entry_line(l.gesture, l.amount, l.curve, l.selection, length, l.points));
        }
        return true;
    }
    // The version 3 (lane_header 6) or 4 (7) bytes held (bytes_, size_) as
    // version 6, in place: first a dry run, which checks them (*valid false on
    // any fault) and that they fit (false if not: kCapacity bytes; a lane's
    // line never outgrows its runs, but for build or break lanes saved with
    // none); then the old bytes move to the end of bytes_ and each variation,
    // read whole, is written from the front, never over bytes not yet read.
    bool to_v6(u32 lane_header, bool* valid) {
        *valid = false;
        if (size_ < 4 || bytes_[0] != 'V' || bytes_[1] != 'R') return false;
        const u32 count = bytes_[3], shift = kCapacity - size_;
        if (count > kMaxVariations) return false;
        Draft d;
        u32 in = 4, out = 4;
        bool fits = true;
        for (u32 n = 0; n < count; ++n) {
            if (!read_old(bytes_, size_, &in, lane_header, d)) return false;
            out += encoded_size(d);
            if (out > shift + in) fits = false;
        }
        if (in != size_) return false;
        *valid = true;
        if (!fits) return false;
        move(shift, 0, size_);
        in = shift + 4, out = 4;
        for (u32 n = 0; n < count; ++n) {
            read_old(bytes_, kCapacity, &in, lane_header, d);
            write(out, d);
            out += encoded_size(d);
        }
        bytes_[0] = 'V', bytes_[1] = 'R', bytes_[2] = static_cast<u8>(kVersion), bytes_[3] = static_cast<u8>(count);
        size_ = out;
        if (!parse_all(bytes_, size_)) return *valid = false, false;  // never: read_old checked every byte
        return true;
    }

    u8 bytes_[kCapacity] = {};
    u32 size_ = 0;
    Variation list_[kMaxVariations] = {};
    u32 count_ = 0;
    u32 generation_ = 0;
    bool kept_ = false;
};
static_assert(sizeof(Point) == 3, "a stored point is three bytes");

// Build 047 (format version 8): the reach list of a window pinned to the join
// into entry `entry` of a song of `entries` entries, from `offset` for `length`
// steps. `scenes` (kNoScene for none known) and `steps` are the song's scene
// and length in steps at each entry. For each later join j = 1.. the window
// crosses (start_j, the steps from the join into `entry` to the join into
// entry + j, is before the window's end) up to kMaxReach and never past the
// song's last entry (no wrap, even in a song that loops): scene[j-1] is the
// scene of entry + j and start[j-1] its start_j. Returns how many (0: the
// window ends in its own section).
inline u32 reach_of(const u8* scenes, const u32* steps, u32 entries, u32 entry, i32 offset, u32 length, u8* scene, u16* start) {
    const i32 end = offset + static_cast<i32>(length);
    u32 from = 0, count = 0;  // from: the steps from the join into `entry` to the next join
    while (count < kMaxReach && entry + count + 1 < entries) {
        from += steps[entry + count];
        if (static_cast<i32>(from) >= end || from > kMaxReachStart) break;
        scene[count] = scenes[entry + count + 1] < kSceneCount ? scenes[entry + count + 1] : kNoScene;
        start[count] = static_cast<u16>(from);  // build 063: from <= kMaxReachStart here
        ++count;
    }
    return count;
}

// Build 058 (format version 10: "I'd like to be able to have another variation span the
// entire song, despite it being put at the first instance of scene 2"): the backward reach
// list, the mirror of reach_of. The window starts -offset steps before its join (offset < 0),
// so it reaches entry `entry - j` when start_j <= -offset (build 064: was <; a window starting exactly on that entry's start plays it), start_j being
// the steps from that join forward to the join into `entry` (the summed steps of entries
// entry - j .. entry - 1). Up to kMaxReach and never before entry 0 (no wrap, even in a song
// that loops, as reach_of does not wrap). scene[j-1] is the scene of entry - j and start[j-1]
// its start_j. Returns how many (0: the window starts in its own section or the one before it;
// the section before needs no record: window_time reads its length live). `length` and `entries`
// are as reach_of's, for the same call; the window's end does not matter going back.
inline u32 back_reach_of(const u8* scenes, const u32* steps, u32 entries, u32 entry, i32 offset, u32 length, u8* scene, u16* start) {
    (void)length;
    u32 from = 0, count = 0;  // from: the steps from the join into entry - count - 1 to the join into `entry`
    // Build 064 (the "from start" length: a variation spanning the song from its very start was silent for its whole
    // first scene): the window's start is -offset steps before the join, and entry - j holds it when its own start, start_j
    // steps back, is AT OR before it: start_j <= -offset. 058 recorded only start_j < -offset, so a window starting exactly
    // on an entry's start never played that entry. (Forward, reach_of's `from >= end` break is right: a window ending
    // exactly on a join does not play any of the entry after it.) offset >= 0: the window starts in its own section.
    while (offset < 0 && count < kMaxReach && entry < entries && count < entry) {
        from += steps[entry - count - 1];
        if (static_cast<i32>(from) > -offset || from > kMaxReachStart) break;
        scene[count] = scenes[entry - count - 1] < kSceneCount ? scenes[entry - count - 1] : kNoScene;
        start[count] = static_cast<u16>(from);
        ++count;
    }
    return count;
}

// Keeps the windows of song `song`'s variations across the song's joins after the
// song is edited (build 047): each placed variation's reach list made again from
// the song as it now is (reach_of; build 058: and back_reach_of) and put back if it changed; its length is kept.
// Where the song now ends sooner the reach shrinks and the window simply plays
// where entries exist; where a section grew past a window's end or a later entry
// came, it grows. Returns how many changed (a put that doesn't fit leaves it as it was).
inline u32 reach_repin(Store& store, u32 song, const u8* scenes, const u32* steps, u32 entries) {
    u32 changed = 0;
    for (u32 i = 0; i < store.count(); ++i) {
        const Variation& v = store[i];
        if (v.song != song || v.unplaced || v.entry >= entries) continue;
        u8 scene[kMaxReach] = {};
        u16 start[kMaxReach] = {};
        const u32 reach = reach_of(scenes, steps, entries, v.entry, v.offset, v.length, scene, start);
        bool same = reach == v.reach;
        for (u32 j = 1; same && j <= reach; ++j) {
            u32 s = 0, t = 0;
            store.reach(v, j, &s, &t);
            same = s == scene[j - 1] && t == start[j - 1];
        }
        // Build 058: the backward list too, by the same test, in the one put.
        u8 back_scene[kMaxReach] = {};
        u16 back_start[kMaxReach] = {};
        const u32 back = back_reach_of(scenes, steps, entries, v.entry, v.offset, v.length, back_scene, back_start);
        same = same && back == v.back;
        for (u32 j = 1; same && j <= back; ++j) {
            u32 s = 0, t = 0;
            store.back(v, j, &s, &t);
            same = s == back_scene[j - 1] && t == back_start[j - 1];
        }
        if (same) continue;
        Draft d = {};
        store.draft(i, d);
        d.reach = reach;
        for (u32 j = 0; j < kMaxReach; ++j) d.reach_scene[j] = j < reach ? scene[j] : kNoScene, d.reach_start[j] = j < reach ? start[j] : 0;
        d.back = back;
        for (u32 j = 0; j < kMaxReach; ++j) d.back_scene[j] = j < back ? back_scene[j] : kNoScene, d.back_start[j] = j < back ? back_start[j] : 0;
        if (store.put(i, d)) ++changed;
    }
    return changed;
}

// Where variation v sits in song `song` as it is now: the entry whose join
// (the join into that entry) holds it, or -1 when it has none. `scenes` is the
// song's scene at each of its `entries` entries (read_song, or the song page's
// view). The join into entry j has scene_of(j - 1) before it (the last entry
// for j = 0) and scene_of(j) after it.
//   - scenes not known (version 1 data): its entry, where it is (if the song
//     still reaches it);
//   - its join still has its scenes: its entry;
//   - at the song's start (entry 0) with the song still starting on its scene
//     after: the start, whatever the song now ends on;
//   - else: the join with the same scenes nearest its old one (the later of
//     two as near: entries are more often inserted before than deleted), or
//     none (-1) with no such join in the song.
inline i32 join_of(const Variation& v, const u8* scenes, u32 entries) {
    if (!entries) return -1;
    auto before = [&](u32 j) { return scenes[(j + entries - 1) % entries]; };
    const u32 sb = v.scene_before, sa = v.scene_after;
    if (sb == kNoScene || sa == kNoScene) return v.entry < entries ? static_cast<i32>(v.entry) : -1;
    if (v.entry < entries && before(v.entry) == sb && scenes[v.entry] == sa) return v.entry;
    // The song's start is found by its first scene alone: the scene before it
    // is only the loop's (and a song that doesn't loop has none), so an edit
    // at the song's end leaves a variation at its start where it is.
    if (v.entry == 0 && scenes[0] == sa) return 0;
    i32 best = -1;
    u32 distance = ~0u;
    for (u32 j = 0; j < entries; ++j) {
        if (before(j) != sb || scenes[j] != sa) continue;
        const u32 d = j > v.entry ? j - v.entry : v.entry - j;
        if (d <= distance) best = static_cast<i32>(j), distance = d;  // <=: the later of two as near
    }
    return best;
}

// Keeps song `song`'s variations on their joins after the song is edited
// (join_of): each moves to its join, taking its join's scenes when they were
// not known (version 1 data), and is placed again if it was unplaced; one
// with no join in the song stays where it is, unplaced: kept and saved, but
// neither played nor shown, until its scenes meet again (version 1 data past
// the song's end is left as it is). Returns how many variations changed.
inline u32 repin(Store& store, u32 song, const u8* scenes, u32 entries) {
    u32 changed = 0;
    auto before = [&](u32 j) { return scenes[(j + entries - 1) % entries]; };
    for (u32 i = 0; i < store.count(); ++i) {
        const Variation& v = store[i];
        if (v.song != song) continue;
        const u32 sb = v.scene_before, sa = v.scene_after;
        const i32 j = join_of(v, scenes, entries);
        if (sb == kNoScene || sa == kNoScene) {
            if (j >= 0 && store.pin(i, static_cast<u32>(j), before(static_cast<u32>(j)), scenes[j], false)) ++changed;
            continue;
        }
        if (j == static_cast<i32>(v.entry)) {
            // At the song's start with another scene at the song's end: takes it as its scene before.
            const u32 now_before = before(v.entry);
            if ((v.unplaced || now_before != sb) && store.pin(i, v.entry, now_before, sa, false)) ++changed;
            continue;
        }
        if (j >= 0) {
            if (store.pin(i, static_cast<u32>(j), sb, sa, false)) ++changed;
        } else if (!v.unplaced) {
            if (store.pin(i, v.entry, sb, sa, true)) ++changed;
        }
    }
    return changed;
}

// Build 049 (2026-11-23): a variation that plays nothing: every lane with a
// gesture muted (an empty lane plays nothing anyway), and at least one such lane.
inline bool muted(const Variation& v) {
    bool any = false;
    for (const Lane& l : v.lanes) {
        if (l.gesture == kNoGesture) continue;
        if (!l.muted) return false;
        any = true;
    }
    return any;
}

// The song page's marks (the SDK's song marks): the joins of song `song`, as
// the song page's cursor counts them (0 the start .. entries the end), that a
// placed variation sits on now (join_of). The join into entry 0 is where the
// song loops: marked at the end its window lies on (the start for one at or
// after it, the end for one before it, both for one across it; build 047). Unplaced variations are not marked; nor is one
// whose scenes no join of the song has any more (the page will mark it
// unplaced when it next opens). `mark(join, muted)` is called once a marked join
// or more; `muted` (build 049): that variation plays nothing (muted()).
template <class Mark>
inline void marked_joins(const Store& store, u32 song, const u8* scenes, u32 entries, Mark mark) {
    for (u32 i = 0; i < store.count(); ++i) {
        const Variation& v = store[i];
        if (v.song != song || v.unplaced) continue;
        const i32 j = join_of(v, scenes, entries);
        if (j < 0) continue;
        const bool quiet = muted(v);
        if (j) {
            mark(static_cast<u32>(j), quiet);
        } else {
            // Build 047 (2026-11-23): only the end(s) its window lies on. A window
            // wholly before the join (at the song's end) was marked at the start too.
            if (v.offset + static_cast<i32>(v.length) > 0) mark(0u, quiet);
            if (v.offset < 0) mark(entries, quiet);
        }
    }
}

// The SDK's song marks for song `song` (extension.cpp song_marks). Build 049: a
// join drawn hollow while everything on it is muted; one variation there that
// plays makes it filled, whichever comes first.
inline void song_marks_of(const Store& store, u32 song, const u8* scenes, u32 entries, opxy::extensions::SongMarks& marks) {
    marked_joins(store, song, scenes, entries, [&](u32 join, bool quiet) {
        if (!quiet) {
            marks.mark(join), marks.unmute(join);
        } else if (!marks.marked(join)) {
            marks.mark_muted(join);
        }
    });
}
}
