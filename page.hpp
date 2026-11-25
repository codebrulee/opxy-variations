// Variations: the variation page, layout C "gesture first"
// (mockups/variation-page-options.html, drawC), with three lanes (build 060; two to 059) as
// mockups/variation-clips.html lays them out (no clips). M4 on the song page
// (without Shift; the SDK labels it "variation" there while Shift is up) opens
// it on the join under the song cursor, and M4 again closes it:
//
//   scene 1 > scene 2     variation 2 of 5                 the join; the title (its number in song order); build 056: the lane's
//                                                          tracks are no longer listed (the track keys' LEDs carry them)
//   a b c                                                  the lane shown white, the others dim (build 060: c; encoder 1's dot is gone, 054)
//   |  scene 1    :   | scene 2      |                       build 049: the timeline, in the shape page's plot: the
//   |      ___    :   |              |                       gesture's own curve (the other lanes dim behind, build 060: sketched from nodes) at the
//   |  ---/   \___ :   |              .                       window's place in the song around the join, the join a
//   |       2     3                                          2 px line, other joins mid grey, the grid, the playhead
//   move bar   gesture    target     amount                  while the song plays here; scene names along the top, bar
//   -2 bars    break      volume     50%                     numbers underneath (encoder 2's dot at the plot's corner)
//
// All text in stock's case (and at stock's sizes, build 022: 10, 12, 30 px): lowercase, as the OP-XY's own labels and messages
// ("step", "beat", "amount", "deleted", "no selection", "song %d").
//
// A variation has one window and three lanes, A, B and C (build 060; two to 059): each its own gesture,
// target, amount and tracks. A plain tap of encoder 1 steps the lane shown a, b, c, a
// (build 022; 020-021's encoder 2; as stock's envelope page flips amp and
// filter; M1 too, plain or with Shift), on both pages, and the letters "a b c" show which (build
// 021: small, top left, the lane shown white, the others dim; build 022: a
// small dark grey dot beside them, encoder 1's colour); encoders 2, 3 and 4
// (gesture, target, amount) and the track keys edit that lane (the LEDs are its tracks). Encoder 1 slides the window by the snap, for
// all three lanes. The page follows stock's language (build 021): encoder turns,
// plain or with Shift, are parameters only; encoder taps (unlabelled, as the
// envelope page's amp/filter tap) flip or step between views; actions are on
// the M keys. While Shift is held the bottom row shows the Shift layer: snap
// (encoder 1's Shift turn: step, beat, bar) and length (encoder 2's Shift
// turn, keeping the window's end, for all three lanes), rate (encoder 3's Shift
// turn: 1x, 2x, 4x, 8x, build 041) and delete over M4. Mute (build 047; build 048 a
// plain tap of encoder 4, 047's Shift tap; build 054 Shift + M1; build 056 plain M2, "mute" over it on both pages): the lane shown is kept and edited but
// plays nothing; its amount reads "muted" and its curve draws dim.
// Build 041: rate is the lane shown's, how many times its line plays across the
// window (store.hpp, DraftLane::rate; stagger.hpp value_late): 1x as before, at
// 2x, 4x, 8x the line repeats in length/N steps. The shape page and every
// other control leave it as it is; a Shift turn clamps at 1x and 8x.
// Build 047 (long form, format version 8): the length goes to 16 bars (256 steps), and a window may run on
// past the section after its join into the entries after it (store.hpp reach_of: up to 15 joins, never past
// the song's last entry); the timeline marks each later join (build 049; a ruler tick before), the title ends "to Z" (the last scene reached) and
// the playhead runs on across them. Edit-time bounds come from the song's shape; extension.cpp repin_all keeps
// the reach in step with song edits (reach_repin).
// Build 046: gate (was stutter) and a new stutter (a note retrigger, no line) join
// the list: break, build, drop, cut in, gate, stutter, fade in,
// fade out, rise, fall, swell, dip, random, wander. Cut in and gate (volume)
// take drop's tracks, random and wander (cutoff) every playing track; choosing
// random or wander rolls new dice (DraftLane::seed), any other change keeps them.
// Encoder 2's tap (build 022; 021's encoder 4: encoder 2 is the gesture, edit
// curve its shape) opens the shape page on the lane shown ("no tracks"
// when it has none); encoder 4's tap does nothing. Shift + M4 deletes the variation (build 030:
// "delete" over M4 in the Shift row), as Shift + M4 deletes
// on stock's song page: the variation goes and the page closes back to the
// song page, which says deleted, no undo. The variations are
// numbered in song order (by join, then by where the window starts; unplaced
// ones not counted), shown in the header ("var 2 of 5"); - and + step through
// them in that order, on to other joins and round the song's ends; closing
// the page then leaves the song page's cursor on the join shown. Every change
// regenerates the points (gestures.hpp) and puts the variation in the store at
// once, so it plays and is saved with the project. A new variation says new in
// the header until its first edit; its lane A is a break on the volume, lanes
// B and C empty ("-", no tracks). The first time M1 shows an empty lane B it takes
// lane A's gesture and amount, with that gesture's target and no tracks
// ("no tracks" until a track key; encoder 2 turns it back to "-"). Lane C never takes one: empty until chosen.
// Drawing (build 060): the lane shown is a smooth outline (paths); the other two are sketches of 31 pieces each
// (nodes), whatever the zoom, so a lane never comes and goes (the page has 4 outlines only). While the
// song plays within the timeline's view, a playhead marks it (bright
// inside the window: this variation is the one heard).
//
// Encoder 2's tap opens the shape page (build 022; build 019's
// "points page", mockups/variation-points.html), one level under, on the lane
// shown; encoder 2's tap there comes back. Its header names it (build 022:
// "edit curve"; build 023 "shape", centred, as the variation page's "variation 2 of 5" names that one). It edits the
// lane's one line (build 024, format version 5: all the lane's tracks follow
// it; build's and break's one after another, stagger.hpp), bright across the
// window, its points on it, a cursor on a step:
//
//   scene 1 > scene 2          shape              3 5   the join; the title; the lane's tracks, all bright ("no tracks" if none)
//                         cutoff                           the target, centred under the title (build 023)
//   a b c                                                  the lane shown white
//   |   :   :   :   |   :   :   :   |                      (build 023: the grid, full height, a line a beat,
//   - - - - - - - - - - - - - - - - -                       the bars brighter; the as set line dashed)
//          (the line, its points, the cursor)
//   ---'---'---'---|---'---'---'---                        the ruler: beats, bars
//   bar 1           bar 2                                  under it, clear of its marks
//   step       value      curve      point
//   13 of 32   -50%       slow 47    2 of 3
//
// Encoder 1 moves the cursor by its snap (Shift: step, beat, bar); 2 sets the
// value at the cursor ("as set", or -100..+100 % of the way to the knob's
// ends), adding a point there if there is none; 3 the curve of the line into
// the point at the cursor; 4 jumps to the next or previous point. Its Shift
// row is snap, and (build 064) "remove" / "press" over encoder 4's column: Shift + its press, or Shift + a step key
// (build 030), removes the point at the cursor (019-029: Shift + M4). M4 does nothing
// here; it never deletes the variation. Step keys put the cursor on a step of the 16
// holding it (their LEDs: the cursor on, points dim); - and + move it by 16.
// Track keys toggle the lane's tracks there (build 057; they did nothing in 024-056), as on the variation page; encoder 1's tap (or M1, plain or with Shift: build 060) the next lane, a, b, c. The target
// is chosen on the variation page only. Editing a gesture's points makes the
// lane custom (DraftLane::edited, its gesture kept): the variation page then
// shows "custom", keeps the points through moves and target changes (a length
// rescales them), and replaces them with the gesture's when its gesture,
// amount or tracks change.
//
// Each gesture bends by its own curve (store.hpp default_curve: rise and fade
// in slow, fall and fade out fast, build and break straight). The page no
// longer bends a whole gesture (build 015-019's Shift + encoder 2): finer
// bending is per point, on the shape page. A lane keeps the curve it was saved
// with (format 4's curve byte), so older variations sound as they did; a new
// gesture takes its own.
//
// Build 032: encoder 3's list is "all" (notes, param 1-8, the fixed targets)
// unless the lane is one track, when that track's engine's own controls join
// it and "track 3" names the target column. Hold encoder 3 + a track key to
// narrow the lane to that track; the same again, or a tap of encoder 3, gives
// its tracks back (focus_track, unfocus).
//
// Build 052 (2026-11-24): both pages adopt stock's engine-page language (mockups/control-rows.html, measured
// against stock's artwork and the device font). A control strip of eight 60 x 20 greyscale ramp cells across the top
// (nodes n_cells; names 10 px at x 5, 125, 245, 365, values 15 px at x 65, 185, 305, 425, left aligned, the right two
// columns in black ink on the light cells), the M-key marks along the bottom, each centred on its key (build 055;
// M1: the lane letters at 3/2 scale (hidden with Shift); M2 (build 056): "mute", both pages; M3: "edit", which opens
// the shape page, and "back" on the shape page; M4: "back" on the variation page, "delete" with Shift, and nothing on the
// shape page - plain M4 is stock's own close, the gesture that opened the page; every mark but "delete" blank with Shift) and the content pushed down between them:
//
//   move bar | -2 bars | gesture | break | target | volume | amount | 50%        the strip (the ramp's cells)
//   scene 1 > scene 2          variation 2 of 5                 2 3 4          the header, baseline 38
//     scene 1      :    scene 2                                                   the timeline (shape page: the plot)
//   a b c          <<  >>        <<  >>        delete                              M1 (lane), M2, M3, M4 (Shift)
//
// The strip's wording is measured at 15 px against 55 px of room: long target names and four gesture names are
// shortened on the page only (words::short_target, kGestureNames); the vocabulary tools/variations.py parses is
// untouched. The window's length is a name and a value ("until" / "[25], 8", "bars" / "2").
//
// The model holds no SDK state: the extension passes the store, what is known
// about the song around the join, the tracks heard in each scene and how to
// name a target on a track. Drawing is a fixed set of nodes and texts,
// recomputed only when the model changes.
#pragma once
#include <opxy/draw.hpp>
#include <opxy/extension.hpp>
#include "gestures.hpp"
#include "points.hpp"
#include "window.hpp"

namespace variations {
using opxy::extensions::PageInput;
using opxy::extensions::PageLeds;
using opxy::extensions::PageTick;
namespace keys = opxy::extensions::keys;
namespace led = opxy::extensions::led;

// The tracks heard in each scene while playing (the sequenced-note event), and
// which are Drum or Drumline tracks: the page's default tracks. The SDK reads no
// scene's patterns from a page, so this is learned from playback; a scene not
// yet heard is unknown (0).
const u32 kScenes = 99;
struct Heard {
    u8 scene[kScenes] = {};
    u8 percussion = 0;
    void forget() {
        for (u8& s : scene) s = 0;
        percussion = 0;
    }
    // A note starting on `track` in `scene`; `to_end` ticks before the scene's
    // boundary. A note in a scene's last two steps may be the next scene's first
    // (the event's context is from before the pass that crosses the boundary): skipped.
    void note(u32 scene_index, u32 track, bool drums, u32 to_end) {
        if (scene_index >= kScenes || track >= kTracks || to_end < 2 * kStepTicks) return;
        scene[scene_index] |= static_cast<u8>(1u << track);
        if (drums) percussion |= static_cast<u8>(1u << track);
        else percussion &= static_cast<u8>(~(1u << track));
    }
};

// The song's entries, read when the page opens: - and + move the page from
// join to join with these (the song can't change while the page is open).
const u32 kEntries = 96;
struct SongShape {
    u32 entries;
    u8 scene[kEntries];
    u32 steps[kEntries];
};

// The song around the join, read when the page opens.
struct Around {
    bool song;           // song playback on, with entries
    u32 song_index, entries;
    u32 entry;           // the entry the join starts (the end of the song: entry 0)
    u32 scene_before, scene_after;
    u32 steps_before, steps_after;  // the sections' lengths
    const SongShape* shape;         // the whole song (null: this join only)
    // Build 025: the song doesn't loop (it stops at its end: SongPosition's
    // loops false), so the join into entry 1 is two: the song's start (a
    // window at or after it) and its end (a window before it); and the page
    // opened on the end (the song page's cursor past the last entry).
    bool stops, at_end;
};

// Build 047: windows to 256 steps (16 bars); past their own section they run on into the entries after it.
// Build 049 (2026-11-23): to 1024 steps (64 bars), a whole song: 512 and 1024 join the fixed lengths
// (the length text already says bars). A line is at most 256 steps (line_length), stretched over a longer window.
// Build 063 (format version 12: "I'd like a song spanning variation to be an option"): to 4096 steps (256 bars):
// 2048 and 4096 join the fixed lengths. choose_length's lists hold these, the scene ends and the scene starts (each
// at most kMaxReach + 1 of them, 95 + 1 now), so they are sized from the three.
const u32 kLengthChoices = 10;
const u32 kLengths[kLengthChoices] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
const u32 kSnapChoices = 3;
const u32 kSnaps[kSnapChoices] = {1, 4, 16};
// The gestures encoder 2 offers, lane A's; lane B's and C's lists start with none
// ("-"), which empties it.
// The fades (build 016) sit with the other volume gestures, just before rise
// and fall.
// Build 040: cut in and gate after drop, random and wander last.
const u32 kGestureChoices = 18;  // build 062: hold ("add in the flat line / silence curve please"), beside the fades
const u32 kOrder[kGestureChoices] = {brk, build, drop, cut_in, gate, stutter, stutter_up, stutter_spring, stutter_down, fade_in, fade_out, hold, rise, fall, swell, dip, random_, wander};
// Build 052 (2026-11-24): the strip's value is 15 px in 55 px of room, so the long names are shortened here.
// Only the page reads this table (the store keeps gesture numbers, tools/variations.py its own long names), so it is
// the display table itself.
// Build 053 (on 052: "random, wander ... don't fit", and "anywhere else we use + or - we could lose the space
// between too"): a value should leave the cell air, not fill it. "random" and "wander" are 'rand' and 'wand',
// "stutter" 'stut' (with its pair), "custom" 'edit' (what it means: the lane's points were edited by hand), and the
// fades and stutters lose the space before their sign.
const char* const kGestureNames[kGestureCount] = {"edit",   "build", "break", "drop", "rise",
                                                  "fall",   "swell", "dip",   "fade+", "fade-",
                                                  "rand",   "wand",  "cut in", "gate", "stut",
                                                  "stut+",  "spring", "stut-",
                                                  "hold"};  // build 062 ("flat line gesture"): not "flat", the curve column's word for no bend
const char kNone[] = "-";
inline const char* gesture_name(u32 g) { return g < kGestureCount ? kGestureNames[g] : kNone; }
// A lane's gesture as the page shows it: custom once the shape page has edited its points.
inline const char* lane_name(const DraftLane& l) { return l.edited ? kGestureNames[custom] : gesture_name(l.gesture); }

// The targets encoder 3 offers (build 025): the notes; "param 1".."param 4",
// each the M1 page's slot on every lane track at once, whatever its engine
// calls it (engine.hpp; offered while the lane has tracks); then the lane
// engines' own controls by name ("tone", "env", "shape", "tremolo"), the
// lowest track's engine first, each name once however many engines have it;
// then the SDK's fixed track targets in the instrument pages' order
// (envelopes, filter, sends, lfo, volume, pan). The list wraps round at its
// ends. Encoder 3's value is the target's name alone (build 024).
static_assert(opxy::extensions::track_target_count > 8 && opxy::extensions::track_targets[0].word == 0 &&
                  opxy::extensions::track_targets[7].word == 7,
              "the engine's M1 controls first in the SDK's list");
// One choice: a word, and for an engine control by name its name.
struct TargetChoice {
    u32 word;
    char name[kNameCapacity];
};
const u32 kMaxNames = 32;  // engine control names a lane lists (8 tracks of 4: every stock M1 page)
const u32 kFixedTargets = opxy::extensions::track_target_count - opxy::extensions::engine_words;
const u32 kMaxChoices = 1 + opxy::extensions::engine_words + kMaxNames + kFixedTargets;
// A target's name on a track, as the SDK gives it (Actions::track_target_label):
// false for no control there. The extension supplies it; tests fake it.
struct Names {
    bool (*name)(u32 track, u32 word, char* text, u32 capacity);
};

// The song's order of its variations, as the page numbers them: by join
// (the song's start, entry 0, first), then by where the window starts, then
// store order. Unplaced variations are not counted.
inline bool earlier(const Variation& a, u32 ia, const Variation& b, u32 ib) {
    if (a.entry != b.entry) return a.entry < b.entry;
    if (a.offset != b.offset) return a.offset < b.offset;
    return ia < ib;
}
inline bool counted(const Variation& v, u32 song) { return v.song == song && !v.unplaced; }
// Store index i's number in song `song` (1-based; 0 when it isn't counted),
// and how many the song counts. An index past the store counts none.
inline u32 song_number(const Store& store, u32 song, u32 i, u32* total) {
    u32 count = 0, ahead = 0;
    const bool known = i < store.count() && counted(store[i], song);
    for (u32 j = 0; j < store.count(); ++j) {
        if (!counted(store[j], song)) continue;
        ++count;
        if (known && j != i && earlier(store[j], j, store[i], i)) ++ahead;
    }
    *total = count;
    return known ? ahead + 1 : 0;
}

// The page's drawing: nodes, texts and palette, as extension.json's page.display.
namespace screen {
const u32 n_other = 0;  // the other lane's motion, dim behind (build 060: unused on the variation page; the shape page's line's first half)
const u32 n_shape = 1;  // the lane's motion
// Build 050 (2026-11-24: the other lane must stay in view): each lane's second half on a wide window (a path
// reaches 256 px; the plot is 432), so both lanes show at any width (build 060: only the lane shown is a path now; the
// others are sketched from nodes, n_sketch). Paths are the first nodes: these took 049's
// join (now node 4, 049's unused n_end) and ruler strip (now the last node).
const u32 n_other_b = 2, n_shape_b = 3;
// Build 049 (2026-11-23): the variation page is a timeline (mockups/variation-timeline.html), not a
// ruler under a centred shape. n_join is the variation's own join, a full plot height line 2 px wide (page_svg.py
// authors it so); n_start is the black strip that covers the static ruler line the shape page draws (the artwork's
// background can't hide it). Build 050: n_join is node 4, n_start the last node (paths 2 and 3 took 2 and 3). The marks (were the window's 28 tiles) are full plot height 1 px
// lines at the other section boundaries; n_bars stay the shape page's ruler marks.
const u32 n_join = 4;
const u32 n_bars = 5, bar_count = 16;
const u32 n_marks = n_bars + bar_count, mark_count = 28;
const u32 n_playhead = n_marks + mark_count;  // the song's position on the shape page's ruler
// The shape page's as set line; the variation page's floor (build 049: none: no shade to keep it from the beat lines).
const u32 n_floor = n_playhead + 1;
// The shape page (its line is the two outlines, nodes 0 and 1: the window's
// first half and second half; the ruler's bar nodes mark its beats and bars).
const u32 n_cursor = n_floor + 1;
const u32 n_points = n_cursor + 1, point_count = 32;          // the line's points
// (Build 024: no dim lines of the lane's other tracks: they follow the one line.)
// Build 021's quiet hints (mockups/variation-page-options.html, drawC): hollow
// handles on the shape (its start, middle and end), so the line looks
// editable; a dot right of the shape's lower right corner, encoder 2's (its
// tap opens the shape page; build 022, was 021's white encoder 4 dot: mid grey,
// the guidebook's encoder 2 colour, in a light outline so it shows on black;
// not on the shape page (build 023: 022's dot after the title there ran into it and
// said little)); the lanes' letters, a and b, the one shown white
// and the other dim, each a ring (its bowl) and a stem (circle and rect nodes:
// the page's 16 text slots are all in use, and a text has one colour).
const u32 n_handles = n_points + point_count, handle_count = 3;
const u32 n_enc2 = n_handles + handle_count;
// Build 060: and c (n_letter_c, below), the third lane's.
const u32 n_ring_a = n_enc2 + 1, n_stem_a = n_ring_a + 1, n_ring_b = n_stem_a + 1, n_stem_b = n_ring_b + 1;
// Build 022: encoder 1's dot beside the letters (its tap switches the lane): dark
// grey, the guidebook's encoder 1 colour, in a mid grey outline; small and quiet.
const u32 n_enc1 = n_stem_b + 1;
// Build 023, the shape page's grid (mockups/variation-page-options.html, "The
// two pages as built"): full-height lines across the plot, one a beat (dim),
// the bars a step brighter (s_grid); only the bars when beats would come
// closer than grid_min px (never at the windows there are: 8 bars is 13.5 px a
// beat), so at most 32 (an 8-bar window's beats; the window ends a step after its last). Then the as
// set line, dashed across the plot (build 049: the variation page's too, and its grid; n_floor unused):
// as_set_count pieces, as_set_pitch px apart from plot_x0, each 8 dashes 3 px
// on and 3 off (a piece is one subpath: its gaps a faint 0.2 px sliver).
const u32 n_grid = n_enc1 + 1, grid_count = 32;
const u32 n_as_set = n_grid + grid_count, as_set_count = 9;
const i32 as_set_pitch = 48;
// Build 049 (2026-11-23): the variation page's window as a faint band behind the grid: band_count strips
// band_width px wide and the plot's height (a node scales only evenly, so a band of any width is strips side by
// side, the last overlapping), in s_band, the darkest grey. Drawn first, behind everything.
const u32 n_band = n_as_set + as_set_count, band_count = 28;
const i32 band_width = 16;
// Build 056: the band stops this far inside its window's ends so a scene marker (1 px, half 0.5) or the song's end line on one stays whole.
const i32 band_inset = 1;
const u32 n_start = n_band + band_count;
// Build 052 (2026-11-24): the control strip's eight cells (authored 60 x 20 black, placed at (60 i + 30, 10) in
// shade ramp[i]). Build 054 ("drop the chevrons...that makes sense"): 052's four M2 and M3 chevrons are gone;
// the - and + keys step the variation and the M keys carry words now.
const u32 n_cells = n_start + 1, cell_count = 8;
// Build 060 (stage B): a third lane, C. Its letter "c" is one filled glyph (a ring open on the right: a ring and a stem
// cannot make it), authored last so no earlier node moves (tools/page_svg.py N_LETTER_C); and the lanes not being edited
// are sketched from nodes, not paths (the page has 4 mutable outlines and the lane shown takes two): sketch_lanes of them
// (kLanes - 1: every lane but the one shown), each sketch_pieces thin ribbons placed end to end with draw::segment.
// 31 pieces: motion() has 32 samples, so 31 joins them exactly, nothing finer exists to draw. Placed in the file before
// the outlines (page_svg.py) so they draw where the other lane's path did, behind the lane shown.
const u32 n_letter_c = n_cells + cell_count;
const u32 sketch_lanes = kLanes - 1, sketch_pieces = 31;
const u32 n_sketch = n_letter_c + 1;
const u32 nodes = n_sketch + sketch_lanes * sketch_pieces;
// A piece is authored sketch_length px long, sketch_thick px thick: segment scales both together (a piece reaching 14 px
// draws 0.6 px thick, one reaching 90 px 3.7), so the length is chosen for the longest piece a plot-high step needs.
const i32 sketch_length = 48;
const u32 paths = 4, points = 64;
// Build 052: the text slots, rebuilt (tools/page_svg.py TEXTS has each one's size, colour and authored place; a slot
// carries one of each, and scaling text is not allowed, so all 16 are spoken for). t_title and t_join are the
// header (baseline 38; build 056: t_tracks, the third header slot, is the M2 mark now); t_label_a and t_label_b the scene names along the timeline (shape page: the bar labels);
// t_number_a the window's bar range and its messages (shape page: the centred target, and its messages); t_mark,
// t_mark_side and t_mark_m2 the three M-key marks (build 056: "mute" over M2 on both pages, "edit" or "back" over M3,
// "back" or, with Shift, "delete" over M4);
// then the strip's four names and four values, column by column.
// Build 054: t_mark_side was the window's end bar number, 10 px. A second 20 px bright slot had to come from
// somewhere - all 16 are spoken for - and the end bar is the one piece of the chart the chart itself already shows
// (the window's band runs to it), which is reported.
// Build 056 ("it's really difficult to understand why you can't resize a variation, it needs to be there"): the
// end bar is back in t_number_a, which now holds the window's bar range ("5-8"; draw_bar_numbers), and the header's
// track list (t_tracks, 12 px) became the third 20 px mark, "mute" over M2 (the track keys' LEDs say the tracks).
enum Text : u32 { t_title, t_mark_m2, t_join, t_label_a, t_label_b, t_number_a, t_mark_side, t_mark,
                  t_name_1, t_name_2, t_name_3, t_name_4, t_value_1, t_value_2, t_value_3, t_value_4, texts };
// s_ring_white, s_ring_mid: a black fill in a white or mid 1 px stroke (the letters' bowls).
// s_grid (build 023): the shape page's bar lines, #555555, between s_dim and s_mid.
// Build 050 (option C1 of mockups/timeline-lines.html): the timeline's own greys, tools/page_svg.py TIMELINE.
// Build 052: s_ramp, #7D7D7D, the strip's one grey the palette lacked (the last entry, the 16th).
enum Shade : u32 { s_black, s_dim, s_mid, s_text, s_bright, s_white, s_ring_white, s_ring_mid, s_grid, s_band,
                   s_beat, s_bar, s_marker, s_join, s_other, s_ramp, palette };
// The strip's cells, left to right, stock's engine-page ramp: #000000 #323232 #555555 #646464 #7D7D7D #969696 #B4B4B4 #CDCDCD.
const u32 ramp[cell_count] = {s_black, s_dim, s_grid, s_mid, s_ramp, s_text, s_join, s_bright};
const i32 unit = opxy::draw::unit;
// Layout, pixels.
// Build 054 ("can we not just use the same playhead we're using on the variation page... that way we can get
// rid of smaller bar below"): no ruler on either page. The shape page's playhead is the variation page's hairline
// now, its bar ticks and the artwork's ruler line are gone, and the bar labels rise into the room they leave.
const i32 left = 16, right = 464;
// Text sizes (build 022): stock's sizes only (its artwork, private/stock-art:
// 20, 10 and 12 px, then 15, 17, 30, 40), as tools/page_svg.py's slots; 9, 11,
// 13 and 26 px looked aliased on the device. header_size is the title's
// ("variation 1 of 5", "shape"), join_size the join's after it.
// Build 024: the message slot 10 px again (023 made it 12 to hold the shape
// page's dim tracks, gone with one line a lane); on the shape page the message
// shows in the target's place, at its 12 px.
// Build 052: the strip's names are 10 px (names_size), its values 15 px (value_size), the M4 mark 20 px (mark_size); the shape
// page's target (t_number_a) is 10 px now, not 12.
const i32 header_size = 12, small_size = 12, label_size = 10, message_size = 10, target_size = 10;
const i32 join_size = 12, value_size = 15, mark_size = 20;
// Build 052: the header's baseline on both pages (was 22), under the strip. The message (variation page, and the "no
// song" screen's "song is off") and that screen's title moved down by the same 18 as the timeline (build 049's 186 and
// 68), to stay centred between the strip and the M row: the message is 17 px under the bar numbers still (they are on
// 187 now, the message on 204, in the M row's free middle between the M2 and M3 marks), not on their row.
const i32 header_y = 38, message_y = 204, no_song_title_y = 86;
// The control strip (build 052; stock's engine pages): cell_width x strip_height cells, a name and a value per
// column (two cells), left aligned at name_x / value_x + column * column_pitch, the name's baseline strip_name_y, the
// value's strip_value_y (16.5 px: Q10 33 * unit / 2). strip_room: what a name or a value may take, pixels (the value's
// inset 65 to the cell's edge 120, 55; the name's the same).
const i32 cell_width = 60, strip_height = 20, column_pitch = 120, name_x = 5, value_x = 65;
const i32 strip_name_y = 10, strip_value_y = 33 * unit / 2, strip_room = 55;
// The M-key marks (build 052; stock's tbuttons.svg anchors): 20 px, baseline m_y, at m1_x, m3_x and m4_x (Q10).
// Build 054: the lane letters at M1, "shape" over M3 and "back" over M4; with Shift, "mute" over M1 (the lane's) and
// "delete" over M4. Build 056: "mute" over M2 (t_mark_m2). The words are t_mark_side, t_mark and t_mark_m2.
const i32 m_y = 215;
// Build 055 (2026-11-24: "the positioning of the m text is odd ... it's meant to be central to the relevant
// button"): stock's own four M labels are CENTRED on their keys (tbuttons.svg: "ab" 28.78, "clear" 153.65, "lisp"
// 294.54, "random" 406.16, left anchors, measured widths 22.34, 43.41, 30.62, 70.03 at 20 px), so the keys' centres
// are 39.95, 175.35, 309.85 and 441.18 px (Q10 below; pitch about 133.7). The marks are drawn with words::centred_x
// on them, no longer left aligned at 052's anchors. m2_x carries "mute" from build 056.
const i32 m1_x = 3995 * unit / 100, m2_x = 17535 * unit / 100, m3_x = 30985 * unit / 100, m4_x = 44118 * unit / 100;
// The lane letters (build 021, in place of 020's large "lane a"): 12 px
// glyphs, a then b, baseline-left at (letters_x, letters_y) on both pages
// (build 022: under the join's scenes); letter_pitch apart.
// letters_width: the letters and encoder 1's dot, enc1_dx right of their start.
// Build 052: the letters moved down to the M1 anchor (letters_x Q10, baseline letters_y), drawn at letters_scale (3/2)
// so they read against the 20 px marks: letter_pitch, letters_width and enc1_dx are as authored at scale 1 and
// multiplied by it (pitch 18, dot 39 px right).
const i32 join_x = 16, join_width = 150;  // the header's scenes, top left (below)
const i32 letters_y = m_y, letter_pitch = 12, letters_width = 30, enc1_dx = 26;
const i32 letters_scale = 3 * unit / 2;
const i32 ring_r10 = 26, x_height10 = 62, stem_a10 = 62, stem_b10 = 90, stem_w10 = 6;  // tenths of a pixel, as tools/page_svg.py
// Build 055: the lane letters are centred on M1's centre as the mark words are. As draw_letters lays them out
// (scale Q10), the drawn width is letter_pitch * scale (b's start) plus one letter's own width, its bowl's diameter
// plus its stem (2 * ring_r10 + stem_w10 tenths, times scale): 18 + 8.7 = 26.7 px at 3/2. They start half of it left
// of m1_x.
// Build 060: three letters, c a pitch right of b and as wide as a bowl (its ring is open, so no stem): the row is
// (kLanes - 1) pitches and one letter's width.
// Build 062 ("the c has a bigger gap next to b than a does to b though, can it be made equal"). A uniform pitch
// spaces the letters' *origins*, not their ink, so c was placed by the gap instead.
// Build 064 (on 062: "c is still just a little too far over to the right"): 062 used the nominal radii. MEASURED
// off the built artwork (px at scale 1, from each letter's origin X): the bowls are circles with a 1 px stroke, so they
// reach +/-3.1 from their centres (ring_r10 + 5 tenths), not 2.6; the stems +/-0.3; and c (an annulus open on the
// right) is asymmetric about its centre, -3.0882 on its left but only +1.9926 on its right. That made 062's gaps
// a-b 5.80 px and b-c 5.01 px, and the row's ink centred at 14.15 where letters_x centred on 14.70 (0.55 px left of
// M1's key). Now everything below is in hundredths of a pixel at scale 1, derived from those extents; the row is
// centred on its real ink; and b-c is set tighter than a-b by c_tighten100, because a-b is two vertical stems facing
// across a constant gap while b-c is a bowl facing an arc, where the space opens above and below, so round shapes are
// set tighter than straight ones (chosen: 1.8 px less, b-c 4.0 px against a-b 5.8, a pixel tighter than 062's 5.01).
const i32 bowl_ink100 = (ring_r10 + 5) * 10;                       // a bowl's ink radius: 310
const i32 c_left100 = 309, c_right100 = 199;                       // c's ink left and right of its centre (3.0882, 1.9926)
const i32 a_stem_right100 = (2 * ring_r10 + 5 + stem_w10 / 2) * 10;  // a's stem: 600
const i32 a_right100 = a_stem_right100 > 2 * bowl_ink100 ? a_stem_right100 : 2 * bowl_ink100;  // a's rightmost ink, its bowl: 620
const i32 b_origin100 = letter_pitch * 100;                        // b's stem sits on its origin: 1200
const i32 letters_gap100 = b_origin100 - a_right100;               // a-b: 580
const i32 b_right100 = b_origin100 + 2 * bowl_ink100;              // b's bowl, its rightmost ink: 1820
const i32 c_tighten100 = 180;                                      // b-c tighter than a-b (see above)
const i32 c_gap100 = letters_gap100 - c_tighten100;                // b-c: 400
const i32 c_centre100 = b_right100 + c_gap100 + c_left100;         // c's glyph centre: 2529
const i32 letters_left100 = 0;                                     // a's bowl, the row's leftmost ink
const i32 letters_right100 = c_centre100 + c_right100;             // 2728
// Centred on M1's key by the real ink, not by the origins or nominal radii.
const i32 letters_x = m1_x - (letters_left100 + letters_right100) * letters_scale / 200;
const i32 enc2_dx = 14;  // encoder 2's dot: right of the shape's lower right corner
const i32 names_size = 10;
// The shape page's plot: the window's steps across (step 0 at plot_x0, the
// window's end, step L, at plot_x1; build 028: was its last step L - 1, which put
// the middle bar line right of 240, at 247 for 2 bars), the value up (0 at
// plot_y1, 255 at plot_y0). Step s at plot_x0 + 432 s / L: the middle bar at 240.
// Build 023: the target sits above it, under the title (points_target_y), as
// mockups/variation-page-options.html's "The two pages as built" draws it.
// Build 024: it starts at y 56 (was 48), padding under the lane letters.
// Build 025: its foot at y 148 (was 156), more room between it and the bar
// labels and the playhead's ruler below.
// Build 052: the plot 16 px down (was 56..148, centre 102), under the strip and the header at 38.
const i32 plot_x0 = 24, plot_x1 = 456, plot_y0 = 72, plot_y1 = 164, plot_cy = 118;
const i32 half_width = (plot_x1 - plot_x0) / 2;          // each outline draws half the plot
const i32 cursor_half = 50;                              // the cursor line, authored 1 x 100 centred (4 px past the plot)
const i32 point_half = 2;                                // a point, authored 4 x 4 centred
// The header (build 022), the same on both pages: the page's title centred
// ("variation 1 of 5", "shape"); the join's scenes top left at join_x (the
// join slot, t_join: the letters are nodes), shortened to end title_gap before
// the title (join_width at most); the lane letters under the scenes (letters_x,
// letters_y); the tracks (on the shape page the track) top right, or "no tracks"
// (build 023; gone in build 056: the LEDs say it, and the strip says "tracks / none"). The ruler's labels
// on the shape page sit under its marks (build 052: the marks rise from the ruler line instead of straddling it, so
// they end at 184 and the labels' ascenders, about 184.8, clear them).
// Build 023: the shape page's target centred under the title (y 34, as the
// mockup), the message in its place while it shows (it was at 38).
// Build 052: the target (10 px now, in t_number_a) at 50.
// Build 054 ("keep the bar text but raise it up a little so it has breathing room"): with the ruler and its ticks
// gone the labels sit in the middle of the room between the plot's foot (164) and the M marks' ascenders (about 201):
// baseline 186, their ink 178.5 to 186, about 14 px clear each way.
const i32 points_target_y = 50, points_message_y = points_target_y, bar_label_y = 186;
const i32 grid_min = 12;
// Build 049 (2026-11-23): the variation page's timeline (mockups/variation-timeline.html) in the plot's
// rectangle. The view (the steps from the join the plot shows) is held in Q6 steps (view_q a step): at 2048 steps
// across 432 px a step is a fifth of a pixel and the products below stay inside 32 bits. Its span is the smallest of
// span_min * 2^k holding twice the window, up to span_max; it eases to its target at ease_rate a second (the mockup's 12).
const i32 view_q = 64, span_min = 32, span_max = 2048, ease_rate = 12;
// The scene names along the plot's top (baseline tl_y0 + name_y, above it from build 050, name_inset right of the section's start, left
// aligned; a section showing less than name_room px has none) and the bar numbers under its foot (baseline plot_y1
// + number_y; a number's left edge number_inset right of its line, at most number_slack; lines at least number_gap
// px apart carry one).
const i32 name_y = -8, name_inset = 8,  /* build 051: more room from the line; was 4 */ name_room = 44, number_y = 13, number_inset = 4 /* build 056 (the join line obscured a number at the join); was 2 */, number_slack = 4, number_gap = 16;
// Build 050 (option C1): the variation page's timeline sits tl_dy px below the shape page's plot, its scene
// names above it (baseline tl_y0 + name_y), clear of the lane letters; the join and the scene markers rise to them.
// Build 052: tl_dy 10 (was 8): the plot's 16 and this 10 make the timeline 18 px lower than before, so the scene
// names' baseline stays 74 and the bar numbers' 187.
// Build 053 (on 052: "we could raise the chart a bit, there's an uneven amount of space between scene 1 and the
// scene 1 > scene 2 text"): tl_dy 4, the chart 6 px up. The scene names' baseline is 68 and the bar numbers' 181, which
// evens the gaps around the chart (header's ink to the names 22 px, the numbers to the M marks 20).
const i32 tl_dy = 4, tl_y0 = plot_y0 + tl_dy, tl_y1 = plot_y1 + tl_dy, tl_cy = plot_cy + tl_dy;
// The curve is a ribbon of at most 250 px (a path reaches 64 either side of a node at scale 2: 256 px, less the rounding); narrower than
// curve_min px (a window only transiently that small in the view) it is left out.
const i32 curve_reach = 250, curve_min = 8;
// (Build 023's bottom row justified like stock's M-key labels, column_2 and column_3, is gone: build 052's strip
// is left aligned at fixed anchors.)
const i32 title_gap = 12;
// The widest each text may be, pixels, by words::width (build 026: the
// device's own measure; the rooms below were chosen with the estimate). The message line: no wider than
// "ENTRIES 1 > 2" (67 by the estimate), which read well on the device; "ONLY
// THIS JOIN" (75) was squeezed (both in capitals, build 005; then 10 px); at
// 10 px and build 024's estimate they are 72.4 and 81.3, so 76 (023's 12 px: 88).
// The others: their room on the page (a column 100 px, so justified columns
// stay 10 px apart). target_width: the target under the gesture and under the
// shape page's title (build 024: fitted there too).
const i32 message_width = 76;
const i32 header_width = 200, tracks_width = 90, numbers_width = 440, label_width = 200;
const i32 target_width = 200;
// Texts hold at most 23 characters (draw::text_capacity less the terminator).
const u32 text_chars = 23;
}
using PageFrame = opxy::draw::Frame<screen::paths, screen::points, screen::nodes, screen::texts, screen::palette>;

namespace words {
inline void put(char* out, u32& n, const char* s) {
    while (*s && n + 1 < opxy::draw::text_capacity) out[n++] = *s++;
    out[n] = 0;
}
inline void number(char* out, u32& n, u32 value) {
    char digits[10];
    u32 count = 0;
    do { digits[count++] = static_cast<char>('0' + value % 10); value /= 10; } while (value);
    while (count && n + 1 < opxy::draw::text_capacity) out[n++] = digits[--count];
    out[n] = 0;
}
// A text's width in Q10 pixels at `size`, estimated: the fallback where the
// device's own measure is missing (build 026: words::width below measures;
// builds 021-025 placed centred and right-aligned texts by this alone, and on
// hardware it kept missing the centre). Widths are
// per mille of the size, near UniversNextPro-Light's (capitals generously, as
// the message line's room was measured with them; lowercase by letter, with
// the same margin). Build 023: plus a tracking of half a pixel a glyph, at
// every size. 022's long title ("variation 1 of 5", 16 glyphs) showed right of
// the centre line and 022's "edit curve" ran into the dot 7 px after its estimated
// end, while the short one-word gesture and target looked centred: the
// device's glyphs run wider than the table by about a fixed amount each, which
// a long text adds up. Half a pixel puts that title's end 5 px further on
// (the dot was 4.5 px clear) and the title 4 px further on each side of 240;
// the 5 or 6 glyph gesture and target move about a pixel. A calibration from
// those two sightings, not a measurement.
// Build 024: on the device the 30 px gesture still sat right of the centre
// line (the 12 px title and target centred, checked against the shape's middle
// handle at x 240): the device's 30 px glyphs run wider than the table. At 30
// px and up the table's widths count large_scale per mille (the 12 px
// calibration as 023). Also a calibration from one sighting: say which way and
// by about how much if the gesture is still off.
// Build 025: at 1.2 the 30 px gesture sat LEFT of centre (023's 1.0 sat right):
// the midpoint, 1.1. Again one sighting each way, not a measurement.
const i32 large_size = 30, large_scale = 1100;
// Build 024 also refits the tracking (from 023's half a pixel a glyph): on the
// shape page the short 12 px "shape" and "notes" sat LEFT of centre (023's
// estimate too wide for them) while the long "variation 1 of 5" and "volume"
// were centred. So width = table + a * glyphs + b, at every size: a = 3/4 px,
// b = -4 px keeps "variation 1 of 5" (16 glyphs: +8 px, as 023) where it is
// and makes 5-glyph words 2.75 px narrower (they move about 1.4 px right).
// (A smaller a with a positive b, as first suggested, would widen short words
// and move them further left.) An empty text is 0 wide.
const i32 track_q4 = 3, offset_px = -4;
inline u32 length(const char* s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}
inline i32 estimate(const char* s, i32 size) {
    i32 w = 0, glyphs = 0;
    for (; *s; ++s) {
        const char c = *s;
        // a-z: a b c d e f g h i j k l m n o p q r s t u v w x y z
        // Univers Next Light's lowercase, widened by the capitals' margin here
        // (7 %), so the message line's room means the same in either case.
        static const unsigned short lower[26] = {560, 600, 510, 600, 570, 320, 600, 600, 250, 250, 540, 250, 900,
                                                 600, 600, 600, 600, 360, 480, 340, 600, 510, 790, 510, 510, 470};
        const i32 m = c == ' ' || c == ',' ? 280 : c >= '0' && c <= '9' ? 560 : c >= 'a' && c <= 'z' ? lower[c - 'a']
                    : c == 'I' ? 260 : c == 'M' || c == 'W' ? 820
                    : c == '%' ? 800 : c == '-' || c == '/' || c == '.' ? 340 : c == '>' ? 600 : 640;
        w += m;
        ++glyphs;
    }
    if (!glyphs) return 0;
    if (size >= large_size) w = w * large_scale / 1000;
    const i32 q = w * size * screen::unit / 1000 + glyphs * track_q4 * screen::unit / 4 + offset_px * screen::unit;
    return q > 0 ? q : 0;
}
// Build 026: a text's width in Q10 pixels at `size` as the device lays it out:
// stock's own measure (opxy::draw::text_width: its glyphs' advances and
// kerning in whole pixels, as stock's centred and right-aligned texts use;
// notes/text-metrics-trace.md), so centred and right-aligned texts sit where
// stock would put them. The estimate above only where there is no measure
// (host tests without fake metrics). An empty text is 0 wide.
// Build 028: below 15 px (stock's hinted sizes, 8d5ff090) the device draws a
// text wider than it measures. Build 027's probe on hardware
// (notes/variations-page-026.md), drawn width less measured width:
// - "variation 1 of 5": about +11.5 px over 16 glyphs
// - "param 1": about +4.5 px over 7 glyphs
// - "shape": about +3 px over 5 glyphs
// - the 10 px probe readout: about +15 px over 19 glyphs
// That is about 3/4 px for each glyph after the first. So at those sizes the
// width counts hinted_track_q4 quarter pixels a glyph after the first on top of
// the measure. 30 px (unhinted) drew as measured (the gesture), so it stays
// the measure alone. A calibration from those photos, not a traced cause.
const i32 hinted_below = 15, hinted_track_q4 = 3;
inline i32 glyphs(const char* s) {
    i32 n = 0;
    for (; *s; ++s) n += (static_cast<unsigned char>(*s) & 0xc0) != 0x80;  // UTF-8 lead bytes
    return n;
}
inline i32 width(const char* s, i32 size) {
    const i32 measured = opxy::draw::text_width(size, s);
    if (measured < 0) return estimate(s, size);
    const i32 n = glyphs(s);
    const i32 track = size < hinted_below && n > 1 ? (n - 1) * hinted_track_q4 * screen::unit / 4 : 0;
    return measured * screen::unit + track;
}
// Where a text centred on `centre` (Q10) starts: as stock centres (8d5493f2),
// x = centre - w / 2 in whole pixels when measured.
inline i32 centred_x(i32 centre, const char* s, i32 size) {
    const i32 w = width(s, size);
    return centre - (w % screen::unit ? w / 2 : (w / screen::unit / 2) * screen::unit);
}
// Where a text ending at `end` (Q10) starts.
inline i32 ending_x(i32 end, const char* s, i32 size) { return end - width(s, size); }
// `s` fitted to `room` (Q10 pixels) at `size`, into `out` (text_capacity):
// whole when it fits (every name the SDK's track_targets give does, in a
// column: tests/page.cpp); else as many of its first words as fit ("filter
// release" -> "filter"; build 023 cut names short, "env amou"); else cut.
inline void fit(char* out, const char* s, i32 room, i32 size) {
    u32 n = 0;
    out[0] = 0;
    put(out, n, s);
    if (width(out, size) <= room && n <= screen::text_chars) return;
    for (u32 k = n; k-- > 1;)
        if (out[k] == ' ') {
            out[k] = 0;
            if (width(out, size) <= room && k <= screen::text_chars) return;
        }
    // Build 059 ("can we add an elipsis or something to cover ourselves"): dropping whole trailing words above
    // leaves text that is true as far as it goes, but cutting into a word leaves a different word ("rando" for
    // "random"), which is how an unshortened engine control name would hide itself. A cut word is marked, with the
    // mark's room kept back first. Where not even one character and the mark fit, the mark is dropped rather than
    // pushed past the cell: a cut with no room to say so is better than text over its room.
    const char mark[] = "...";
    const i32 mark_w = width(mark, size);
    const u32 mark_n = length(mark);
    n = length(out);
    while (n > 1 && (width(out, size) + mark_w > room || n + mark_n > screen::text_chars)) out[--n] = 0;
    if (width(out, size) + mark_w <= room && n + mark_n <= screen::text_chars) return put(out, n, mark);
    while (n > 1 && (width(out, size) > room || n > screen::text_chars)) out[--n] = 0;
}
// Build 052 (2026-11-24): the strip's value is 15 px in 55 px of room, and these SDK target names are the only ones
// over it; the page shows them shortened (a display only: target_text_ and tools/variations.py keep the long names).
// A trailing " !" (build 039's CPU warning) follows the short name.
inline void short_target(char* out, const char* name) {
    // Build 053 (on 052: "volume", "env amt" and the rest "don't fit"). A value that fills 90 % of its cell reads
    // as crowded, so the names come down to about two thirds of it: the envelope's four words are their three letter
    // forms everywhere ("atk" 20 px, "dec" 25, "sus" 22, "rel" 17), the filter's keep "flt" in front, and the rest are
    // as short as they can be and still be read. Build 039's warning (" !", 13 px) then fits beside any of them, so
    // 052's warned-only pair is gone.
    static const char* const names[][2] = {
        {"attack", "atk"},           {"decay", "dec"},              {"sustain", "sus"},            {"release", "rel"},
        {"volume", "vol"},           {"length", "len"},             {"resonance", "reso"},         {"env amount", "env"},
        {"filter attack", "flt atk"},{"filter decay", "flt dec"},   {"filter sustain", "flt sus"}, {"filter release", "flt rel"},
        {"key track", "key trk"},    {"lfo speed", "lfo spd"},      {"lfo amount", "lfo amt"},     {"ext send", "ext"},
        {"tape send", "tape"},       {"fx i send", "fx i"},         {"fx ii send", "fx ii"},
        // Build 059 ("somehow the word random has found it's way in again, and is too long, it should be rand
        // probably"): these are an engine's own control names, not SDK targets, so they reach the strip unshortened.
        // "random" measures 51 px of the 55 px cell - it fits, but it fills it, which 053 settled against. They take
        // the same short forms the gesture list uses (kGestureNames), so one word means one thing on the page.
        {"random", "rand"},          {"wander", "wand"}};
    u32 n = 0;
    out[0] = 0;
    const u32 whole = length(name);
    const bool warned = whole > 2 && name[whole - 2] == ' ' && name[whole - 1] == '!';
    const u32 base = warned ? whole - 2 : whole;
    auto matched = [&](const char* const* pair) {
        u32 k = 0;
        while (k < base && pair[0][k] && pair[0][k] == name[k]) ++k;
        return k == base && !pair[0][k];
    };
    for (const auto& pair : names)
        if (matched(pair)) {
            put(out, n, pair[1]);
            if (warned) put(out, n, " !");
            return;
        }
    // "param 1".."param 4" (an engine's M1 slots, name_target_text) are "par 1".."par 4": 54 px whole, 37 short.
    if (base > 6 && name[0] == 'p' && name[1] == 'a' && name[2] == 'r' && name[3] == 'a' && name[4] == 'm' && name[5] == ' ') {
        put(out, n, "par ");
        for (u32 k = 6; k < whole; ++k) out[n++] = name[k];
        out[n] = 0;
        return;
    }
    put(out, n, name);
}
}

// Build 052: one column of the control strip: its name (10 px) and its value (15 px), each fitted to strip_room (words::fit:
// whole, else as many words as fit), left aligned at the column's anchors (screen::name_x, value_x, column_pitch).
template <class F> void strip_column(F& frame, u32 column, const char* name, const char* value) {
    using namespace screen;
    namespace draw = opxy::draw;
    char out[draw::text_capacity];
    const i32 shift = static_cast<i32>(column) * column_pitch;
    words::fit(out, name, strip_room * unit, names_size);
    draw::text(frame, t_name_1 + column, (name_x + shift) * unit, strip_name_y * unit, unit, out);
    words::fit(out, value, strip_room * unit, value_size);
    draw::text(frame, t_value_1 + column, (value_x + shift) * unit, strip_value_y, unit, out);
}
// The strip's cells, both pages (they are the same on both; build 054: 052's chevrons are gone).
template <class F> void place_strip(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    for (u32 i = 0; i < cell_count; ++i)
        draw::place(frame, n_cells + i, (cell_width * static_cast<i32>(i) + cell_width / 2) * unit, strip_height * unit / 2, unit, ramp[i],
                    draw::fill_and_stroke);
}


class VariationPage {
public:
    // Opens on the join `a` names: its first variation, or a new break ending on
    // the join. The new one is put in the store at once (it plays as shown).
    // `names` names targets on tracks (null: positions and fixed labels only).
    void open(Store& store, const Around& a, const Heard& heard, const Names* names = nullptr) {
        around_ = a;
        opened_entry_ = a.entry;
        heard_ = &heard;
        names_ = names;
        message_ = nullptr;
        notice_ = nullptr;
        stored_ = false;
        fresh_ = false;
        shift_ = false;
        heading_ = false;
        shown_x_ = -1;
        points_ = false;
        number_ = total_ = 0;
        snap_ = 2;
        lane_ = 0;
        view_set_ = false;  // build 049: the timeline places its view, at once (nothing to ease from)
        view_snap_ = true;
        ++revision_;
        if (!a.song) return;
        generation_ = store.generation();
        if (!pick_first_here(store)) start_new(store);
    }

    // One input while open; true when the page should close: after a delete
    // (Shift + M4; notice() then says what to tell the song page). The
    // opening gesture (M4 without Shift, taken by the SDK) and mode keys close
    // it without reaching it. Turns: encoder 1 moves, 2 picks the gesture,
    // 3 the target, 4 the amount; with Shift encoder 1 the snap, 2 the length,
    // 3 and 4 nothing. Taps (build 022): encoder 1's the next lane (M1 too; build 060: a, b, c),
    // encoder 2's the shape page; 3's and 4's nothing; with Shift only encoder 4's does anything (build 047: mute).
    bool input(Store& store, const PageInput& in, const Names* names = nullptr) {
        if (names) names_ = names;
        if (in.shift != shift_) {  // the Shift layer, from every input (Shift's own press and release too)
            shift_ = in.shift;
            ++revision_;
        }
        if (!around_.song) return false;
        if (store.generation() != generation_) {  // the project changed under the page
            around_.song = false;
            points_ = false;
            ++revision_;
            return false;
        }
        if (points_) {
            points_input(store, in);
            return false;
        }
        if (in.kind == PageInput::turn && in.step) {
            const i32 dir = in.step > 0 ? 1 : -1;
            switch (in.encoder) {
            case 0:
                if (in.shift) {
                    const i32 s = static_cast<i32>(snap_) + dir;
                    if (s >= 0 && s < static_cast<i32>(kSnapChoices)) snap_ = static_cast<u32>(s), ++revision_;
                } else {
                    move(store, in.step);
                }
                break;
            case 1:
                if (in.shift) choose_length(store, in.step);
                else choose_gesture(store, in.step);
                break;
            case 2:
                if (in.shift) choose_rate(store, dir);
                else choose_target(store, dir);
                break;
            case 3: if (!in.shift) choose_amount(store, in.step * (in.speed >= 2 ? 5 : 1)); break;
            }
        } else if (in.kind == PageInput::press && in.key == keys::m1 + 3 && in.shift) {
            remove_variation(store);  // stock's song page delete, the same key
            return true;
        } else if (in.kind == PageInput::press && in.key == keys::m1 + 1 && !in.shift) {
            // Build 047 (2026-11-23): the lane shown mutes: kept and edited, but it plays nothing
            // (store.hpp kLaneMuted); its amount reads "muted". It was encoder 4's tap (048), Shift + M1 in 054, and
            // is plain M2 from build 056 ("it should be the second item on both variation and edit screens").
            DraftLane& l = lane();
            l.muted = !l.muted;
            commit(store);
        } else if (in.kind == PageInput::press && in.key == keys::m1 + 2 && !in.shift) {
            enter_points();  // build 054: M3 (it was encoder 2's tap); build 056: marked "edit"
        } else if (in.kind == PageInput::press && in.key == keys::m1) {
            // Build 057 (on 056: "I didn't ask for this change?"): Shift + M1 switches lane again, as plain M1 does.
            // It muted in build 054 and does not now: mute moved to M2 in 056.
            switch_lane(store);  // the next lane (build 060: a, b, c): its tracks, LEDs, gesture and target
        } else if (in.kind == PageInput::held && in.encoder == 2) {
            focused_with_hold_ = false;
        } else if (in.kind == PageInput::press && in.key >= keys::track && in.key < keys::track + kTracks &&
                   (in.encoders >> 2 & 1)) {
            focused_with_hold_ = true;  // encoder 3 held: its click (on release) does nothing
            focus_track(store, in.key - keys::track);
        } else if (in.kind == PageInput::click && in.encoder == 2 && !in.shift) {
            if (!focused_with_hold_) unfocus(store);  // a plain tap: the lane's tracks back, "all"
            focused_with_hold_ = false;
        } else if (in.kind == PageInput::press && in.key >= keys::track && in.key < keys::track + kTracks) {
            DraftLane& l = lane();
            focus_[lane_].valid = false;  // tracks chosen by hand: nothing to give back
            l.selection ^= 1u << (in.key - keys::track);
            l.default_tracks = false;
            l.edited = false;  // a gesture's points again, on the tracks as they now are
            commit(store);
        } else if (in.kind == PageInput::press && (in.key == keys::minus || in.key == keys::plus)) {
            // Build 054: - and + alone (052's M2 and M3 chevrons are gone; M3 is "shape" now).
            step_variation(store, in.key == keys::plus ? 1 : -1);
        }
        return false;
    }

    // After the page has gone. Edits are already in the store.
    void close(Store&) {
        around_.song = false;
        points_ = false;
        shift_ = false;
        heard_ = nullptr;
        names_ = nullptr;
    }

    // The lane's tracks on the track keys (both pages: on the shape page all
    // follow the one line). On the shape page (build 071) the step keys are the line's points, 16 to a page: lit,
    // the one the cursor is on brighter.
    void leds(PageLeds& out) const {
        for (u32 t = 0; t < kTracks; ++t)
            out.track[t] = !around_.song || !(lane().selection >> t & 1) ? led::off : missed_ >> t & 1 ? led::dim : led::on;
        for (u32 k = 0; k < 16; ++k) out.step[k] = led::off;
        if (!points_ || !around_.song) return;
        // Build 071 ("the steps could reflect the points on the shape"): key k is the page's k-th point, lit; the
        // point the cursor is on is brighter; keys past the last point are off.
        const DraftLane& l = lane();
        const u32 first = points_page() * 16;
        for (u32 k = 0; k < 16 && first + k < l.count; ++k)
            out.step[k] = l.points[first + k].step == cursor_ ? led::on : led::dim;
    }
    // Build 071: which 16 of the line's points (at most kDraftPoints = 32, two pages) the step keys hold: the page of
    // the point at the cursor, or of the last point behind it when the cursor is between points (page 0 before the first).
    u32 points_page() const {
        const DraftLane& l = lane();
        u32 behind = 0;
        while (behind < l.count && l.points[behind].step <= cursor_) ++behind;
        return behind ? (behind - 1) / 16 : 0;
    }

    // What the page asks the SDK to say over the song page as it closes
    // (deleted), once; null for nothing.
    const char* notice() {
        const char* n = notice_;
        notice_ = nullptr;
        return n;
    }

    template <class F> void draw(const PageTick& tick, F& frame);

    // The song's position after a sequencer pass (the extension calls this
    // each pass): where the playhead goes. Shown while the song plays this
    // song in the section after the join (its entry) or the one before it.
    void follow(const SongPosition& p) {
        heading_ = false;
        if (!around_.song || !p.playing || p.song != around_.song_index || !around_.entries) return;
        const u32 before = (around_.entry + around_.entries - 1) % around_.entries;
        if (p.entry == around_.entry) {
            playhead_ = static_cast<i32>(p.position);
        } else if (p.entry > around_.entry && p.entry - around_.entry <= draft_.reach) {
            // Build 047: a later entry the window runs on into: the song's place from the join, start_j on.
            playhead_ = static_cast<i32>(draft_.reach_start[p.entry - around_.entry - 1] * kStepTicks + p.position);
        } else if (p.entry == before) {
            playhead_ = static_cast<i32>(p.position) - static_cast<i32>(p.length);  // the join is the scene's end
        } else {
            return;
        }
        heading_ = true;
    }

    // For the extension and tests.
    const Draft& draft() const { return draft_; }
    u32 lane_index() const { return lane_; }
    const char* target_text() const { return target_text_; }
    // The target's place in the lane's list (1-based; 0 when the list doesn't
    // offer it) and how many the list offers.
    u32 target_place() const { return target_place_; }
    u32 target_count() const { return target_count_; }
    bool stored() const { return stored_; }
    u32 index() const { return index_; }
    u32 snap() const { return kSnaps[snap_]; }
    const char* message() const { return message_; }
    bool showing() const { return around_.song; }
    bool fresh() const { return fresh_; }
    bool shift() const { return shift_; }
    u32 entry() const { return around_.entry; }
    // Build 025: on the start of a song that doesn't loop (the join into
    // entry 1, the window at or after it): the song page's cursor goes to
    // the song's start, not its end.
    bool at_song_start() const { return split() && start_side(); }
    // Whether - or + moved the page off the join it opened on: closing then
    // leaves the song page's cursor on the join shown (entry()).
    bool moved() const { return around_.song && around_.entry != opened_entry_; }
    // The variation's place in the song's order (1-based; 0: not in the store)
    // and how many the song has (placed ones only).
    u32 number() const { return number_; }
    u32 total() const { return total_; }
    i32 playhead() const { return playhead_x(); }
    // The shape page: whether it shows, the cursor (a step of the window)
    // and the cursor's snap in steps.
    bool on_points() const { return points_; }
    u32 cursor() const { return cursor_; }
    u32 points_snap() const { return kSnaps[points_snap_]; }
    // Build 049: the timeline's target view, steps from the join (aimed at each redraw; the drawn one eases to it).
    i32 view_from() const { return view_from_; }
    i32 view_to() const { return view_to_; }
    // Build 058: how far back the window's start may go now (for tests; lowest()).
    i32 lowest_start() const { return lowest(draft_.length, draft_.offset); }

private:
    DraftLane& lane() { return draft_.lanes[lane_]; }
    const DraftLane& lane() const { return draft_.lanes[lane_]; }
    // The join's first variation in song order (where its window starts), or false.
    bool pick_first_here(const Store& store) {
        i32 first = -1;
        for (u32 i = 0; i < store.count(); ++i)
            if (here(store[i]) && (first < 0 || earlier(store[i], i, store[static_cast<u32>(first)], static_cast<u32>(first))))
                first = static_cast<i32>(i);
        if (first < 0) return false;
        pick(store, static_cast<u32>(first));
        return true;
    }
    // The first time an empty lane B shows for a variation, it takes lane A's
    // gesture and amount (lane B mostly goes with lane A), its target the
    // gesture's own, and no tracks: "no tracks" until a track key, so nothing
    // plays until then. Encoder 2 still turns it back to none. Not from a
    // custom lane A (its points are its own) or an empty one.
    void follow_lane_a(Store& store) {
        if (followed_) return;
        followed_ = true;
        DraftLane& b = draft_.lanes[1];
        const DraftLane& a = draft_.lanes[0];
        if (b.gesture != kNoGesture || b.selection || a.gesture == custom || a.edited || a.gesture == kNoGesture) return;
        b.gesture = a.gesture;
        b.amount = a.amount;
        b.curve = a.curve;
        b.default_target = true;
        b.default_tracks = false;
        const bool fresh = fresh_;
        commit(store);
        fresh_ = fresh;  // not an edit of its own: a new one stays new
    }
    // The next lane shown (build 022: a plain tap of encoder 1, 020-021's
    // encoder 2, as stock's
    // envelope page flips amp and filter by a knob's tap; M1 too). Build 060: a, b, c, a ...
    // An empty lane B takes lane A's gesture the first time it shows; an empty lane C stays empty: showing a lane
    // is not an edit, and C is the spare, so nothing is made for it that the musician did not ask for.
    void switch_lane(Store& store) {
        lane_ = (lane_ + 1) % kLanes;
        if (lane_ == 1) follow_lane_a(store);
        name_target();
        ++revision_;
    }
    // Store index i on the page (on its join already).
    void pick(const Store& store, u32 i) {
        followed_ = false;
        for (u32 k = 0; k < kLanes; ++k) kept_[k].valid = false, focus_[k] = {};  // build 032: a track focus is the page's, per variation shown (060: all three lanes)
        store.draft(i, draft_);
        index_ = i;
        stored_ = true;
        fresh_ = false;
        view_set_ = false;  // build 049: a variation shown: the timeline places its view again
        name_target();
        recount(store);
        ++revision_;
    }
    // Pinned to this join and placed (unplaced ones are kept but not shown).
    bool here(const Variation& v) const {
        return v.song == around_.song_index && v.entry == around_.entry && !v.unplaced;
    }
    // number_ and total_: where the variation shown stands in the song's order.
    void recount(const Store& store) {
        number_ = stored_ ? song_number(store, around_.song_index, index_, &total_) : 0;
        if (!stored_) song_number(store, around_.song_index, store.count(), &total_);
    }
    // The page on the join into `entry` (the song's shape known): its scenes
    // and sections, the ruler and playhead with them. The song page's own
    // cursor follows when the page closes (the extension asks the SDK).
    void aim(u32 entry) {
        const SongShape& s = *around_.shape;
        const u32 before = (entry + s.entries - 1) % s.entries;
        around_.entries = s.entries;
        around_.entry = entry;
        around_.scene_before = s.scene[before], around_.scene_after = s.scene[entry];
        around_.steps_before = s.steps[before], around_.steps_after = s.steps[entry];
        heading_ = false;  // the next pass places the playhead for this join
        view_snap_ = true;  // build 049: the timeline's steps are from this join: nothing to ease from
        ++revision_;
    }
    void start_new(Store& store) {
        Draft& d = draft_;
        d.song = around_.song_index;
        d.entry = around_.entry;
        d.length = 32;
        // Build 025: at the start of a song that doesn't loop, from the start.
        d.offset = split() && !around_.at_end ? 0 : -32;
        // Build 066: a new variation is born inside its own bounds. Two bars before the join is the default, but on
        // a join near the song's start that is before the song begins, and extent() widens the view to wherever the
        // window actually sits - so an out-of-bounds birth put the dead space straight back on screen.
        {
            const i32 lo = lowest(d.length, d.offset), hi = highest(d.length, d.offset);
            if (d.offset < lo) d.offset = lo;
            if (d.offset > hi) d.offset = hi;
        }
        DraftLane& a = d.lanes[0];
        clear_lane(a);
        a.gesture = brk;
        a.amount = 128;
        a.default_tracks = true;
        a.default_target = true;
        clear_lane(d.lanes[1]);
        clear_lane(d.lanes[2]);  // build 060 (stage A): lane C starts empty
        index_ = store.count();
        stored_ = false;
        followed_ = false;
        for (u32 k = 0; k < kLanes; ++k) kept_[k].valid = false, focus_[k] = {};  // build 032: a track focus is the page's, per variation shown (060: all three lanes)
        view_set_ = false;  // build 049: a new variation: the timeline places its view again
        commit(store);
        fresh_ = stored_;  // NEW until the first edit
    }
    // Shift + M4: the variation out of the store; the page
    // then closes and the song page says so. No undo.
    void remove_variation(Store& store) {
        if (stored_) store.remove(index_);
        stored_ = false;
        fresh_ = false;
        message_ = nullptr;
        notice_ = "deleted";
        recount(store);
        ++revision_;
    }
    // - and +: the next or previous variation in song order (song_number),
    // on to other joins and round the song's ends. Not in the store (full):
    // from where it would stand. One alone (or none other): said so.
    void step_variation(Store& store, i32 dir) {
        const Variation* self = stored_ ? &store[index_] : nullptr;
        Variation shown = {};
        shown.entry = static_cast<unsigned char>(draft_.entry);
        shown.offset = static_cast<short>(draft_.offset);
        if (!self) self = &shown;
        const u32 self_index = stored_ ? index_ : store.count();
        i32 best = -1, wrap = -1;
        for (u32 i = 0; i < store.count(); ++i) {
            const Variation& v = store[i];
            if (i == self_index || !counted(v, around_.song_index) || !reachable(v)) continue;
            const bool after = earlier(*self, self_index, v, i);
            const Variation* b = best < 0 ? nullptr : &store[static_cast<u32>(best)];
            const Variation* w = wrap < 0 ? nullptr : &store[static_cast<u32>(wrap)];
            if (dir > 0) {
                if (after && (!b || earlier(v, i, *b, static_cast<u32>(best)))) best = static_cast<i32>(i);
                if (!w || earlier(v, i, *w, static_cast<u32>(wrap))) wrap = static_cast<i32>(i);  // the song's first
            } else {
                if (!after && (!b || earlier(*b, static_cast<u32>(best), v, i))) best = static_cast<i32>(i);
                if (!w || earlier(*w, static_cast<u32>(wrap), v, i)) wrap = static_cast<i32>(i);  // the song's last
            }
        }
        const i32 to = best >= 0 ? best : wrap;
        if (to < 0) return say("no others");
        const u32 entry = store[static_cast<u32>(to)].entry;
        if (entry != around_.entry) aim(entry);
        pick(store, static_cast<u32>(to));
    }
    // Whether - and + can show v: on this join, or another of the song's shape.
    bool reachable(const Variation& v) const {
        return v.entry == around_.entry || (around_.shape && v.entry < around_.shape->entries);
    }
    // The section after's length bounds where the window may end; build 024:
    // the section before's where it may start, so it reaches as far left as
    // right, wholly before the join if need be (023 stopped it ending on the
    // join). A window longer than the section before still reaches the join.
    //
    // Build 025: a song that doesn't loop has no section before its start and
    // none after its end, so on the join into entry 1 the window stays on its
    // side: at the start (offset >= 0) it can't move before the song's first
    // bar; at the end (offset < 0) not past the last entry's end (offset +
    // length <= 0). Its side is where it is (`offset`): nothing moves it across.
    //
    // Build 030: the start side is any window reaching past the song's start
    // (offset + length > 0), not only one starting at or after it. A window
    // made across the start while the song looped (partly in the minus bars)
    // stayed on the end side once loop was turned off, and could never come
    // back to the join (2026-11-23); now the first move puts it on bar 1.
    //
    // Build 031 took the wall away altogether, which was not wanted (2026-11-23):
    // with loop off the window must not reach the other end of the song.
    //
    // Build 032: the side is the one the page was opened on (the song page's
    // cursor at the song's start, or past its end: Around::at_end), its home.
    // There the bounds hold as in 025: from the start not before bar 1, from
    // the end not past the song's end. A window not on its home side (put
    // there while the song looped: wholly or partly before the start, seen
    // from the start) is not stuck: the first turn of encoder 1, either way,
    // brings it onto its home side at the join (bar 1, or ending on the end).
    bool split() const { return around_.stops && song_start(); }
    bool start_side() const { return draft_.offset + static_cast<i32>(draft_.length) > 0; }
    bool home_is_start() const { return !around_.at_end; }
    // Whether a window from `offset` lies wholly on the home side.
    bool at_home(i32 offset, u32 length) const {
        return home_is_start() ? offset >= 0 : offset + static_cast<i32>(length) <= 0;
    }
    i32 lowest(u32 length, i32 offset) const {
        (void)offset;
        if (split() && home_is_start()) return 0;
        u32 reach = around_.steps_before > length ? around_.steps_before : length;
        // Build 058 (2026-11-24: "I'd like to be able to have another variation span the entire song, despite it being
        // put at the first instance of scene 2"; "the dead space is a previous scene"): the start may go back across
        // earlier scenes, as highest() goes forward (047): the summed steps of the entries before the join, up to kMaxReach
        // of them and not before entry 0 (no wrap, even in a song that loops). Never less than the section before or the
        // window's own length, as before.
        if (around_.shape) {
            const SongShape& s = *around_.shape;
            u32 sum = 0;
            for (u32 k = 1; k <= kMaxReach && k <= around_.entry && around_.entry - k < s.entries; ++k) sum += s.steps[around_.entry - k];
            if (sum > reach) reach = sum;
        }
        i32 lo = -static_cast<i32>(reach > kMaxOffset ? kMaxOffset : reach);
        if (lo < -static_cast<i32>(kMaxOffset)) lo = -static_cast<i32>(kMaxOffset);
        // Build 066 ("I'm still able to move it further back than that, and the left side of the screen isn't
        // anchored to the start, which doesn't make sense to me, what is that space?"). The `length` term above is
        // build 024's rule, that a window may always move back by its own length so a break can sit wholly before
        // its join. It never asked whether there is any song back there, so on a join part way in a window longer
        // than the distance to the song's start could begin before the song did - and those steps never play. The
        // section-before branch reckons the time from the entry's own start, so the window is already part way
        // through when the song opens and the shape's first steps simply never happen.
        // Not on entry 0: there the section before the join is the song's last scene arriving by the loop, and it
        // does play (builds 025, 030, 031 and 032 are that case's history). extent() still widens to any offset a
        // variation already holds, so one saved out there before this build still draws.
        if (around_.shape && around_.entry > 0) {
            const i32 start = song_start_at();
            if (lo < start) lo = start;
        }
        return lo;
    }
    i32 highest(u32 length, i32 offset) const {
        i32 hi = static_cast<i32>(around_.steps_after) - static_cast<i32>(length);
        if (split() && !home_is_start()) {
            hi = -static_cast<i32>(length);
        } else if (around_.shape) {
            // Build 047 (long form): on the start side the window's end may pass the section after the join, as
            // far as the end of entry E + k, the furthest entry with k <= kMaxReach (31 from build 049) that the song has (no wrap, even in
            // a song that loops). It still starts inside its own section (offset < steps_after).
            const SongShape& s = *around_.shape;
            u32 end = 0;
            for (u32 k = 0; k <= kMaxReach && around_.entry + k < s.entries; ++k) end += s.steps[around_.entry + k];
            // Build 047 held the start inside its own section (offset < steps_after), on the grounds that a window
            // whose start passed the next join would belong to that join. Build 070 drops it (having had to
            // switch from bars to beats to get near the song's end and still not reach it: "then find it doesn't
            // perfectly fit, when the song is an even numbers of bars...this doesn't seem right"). It was an
            // asymmetry as much as a rule: a window may sit wholly *before* its join (build 024's break), and build
            // 064 let one starting exactly on an entry's start reach that entry, yet forward the start could not
            // even touch the next join - so a window could never end flush on the song's end from a join whose own
            // section was shorter than the window. On his song that left it one bar short however he moved it.
            // The end of the furthest entry reached still bounds it, so the window stays inside the song.
            const i32 far = static_cast<i32>(end) - static_cast<i32>(length);
            if (far > hi) hi = far;
        }
        if (hi > static_cast<i32>(kMaxOffset)) hi = static_cast<i32>(kMaxOffset);
        return hi < lowest(length, offset) ? lowest(length, offset) : hi;
    }
    void move(Store& store, i32 detents) {
        const i32 snap = static_cast<i32>(kSnaps[snap_]);
        const i32 o = draft_.offset;
        if (split() && !at_home(o, draft_.length)) {  // build 032: onto its home side, at the join
            draft_.offset = home_is_start() ? 0 : -static_cast<i32>(draft_.length);
            commit(store);
            return;
        }
        // To the snap's grid first, then by whole snaps.
        const i32 base = (o >= 0 ? o / snap : -((-o + snap - 1) / snap)) * snap;
        i32 to = detents > 0 ? base + detents * snap : (o == base ? base : base + snap) + detents * snap;
        // Build 065 (on a window showing "move +63s" with its band's edge a step off a scene marker): a clamp
        // used to drop the window straight onto the raw bound, which is rarely on any grid - the start cap is
        // steps_after - 1, so moving right landed on 63 steps, three bars and fifteen steps, and the band's edge
        // came to rest one step from the marker. The clamp now lands on the last position the snap allows, and a
        // second press goes the rest of the way to the bound, so a window flush to the song's end is still reachable.
        const i32 lo = lowest(draft_.length, o), hi = highest(draft_.length, o);
        auto on_grid = [snap](i32 v, bool down) {
            const i32 floor_v = (v >= 0 ? v / snap : -((-v + snap - 1) / snap)) * snap;
            return down ? floor_v : (floor_v == v ? v : floor_v + snap);
        };
        // Build 068 (photos still reading "move +63s" on build 065): 065 landed on the grid on the *way* to a
        // bound, then gave the bound itself on the next press - so holding the encoder still ended on step 63, three
        // bars and fifteen steps, which is what he was reporting. The bound is only offered now when it is on the
        // grid itself. `start_cap` is "the last step inside my own section" and is never a musical place to begin,
        // while a bound that is grid-aligned - a window ending flush on the song's end, scenes being whole bars -
        // still lands exactly.
        if (to > hi) {
            const i32 grid = on_grid(hi, true);
            to = grid >= lo ? grid : hi;
        }
        if (to < lo) {
            const i32 grid = on_grid(lo, false);
            to = grid <= hi ? grid : lo;
        }
        if (to == o) return;
        draft_.offset = to;
        commit(store);
    }
    // Lane A's gestures, or lane B's or C's (none first, which empties it: no
    // tracks). From custom any turn starts the list; from none on lane B or C, the
    // tracks follow the gesture unless some were chosen.
    void choose_gesture(Store& store, i32 dir) {
        DraftLane& l = lane();
        const u32 first = lane_ ? 0 : 1, count = kGestureChoices + 1;  // choice 0 is none
        auto gesture_at = [](u32 c) { return c ? kOrder[c - 1] : kNoGesture; };
        u32 at = first;
        while (at < count && gesture_at(at) != l.gesture) ++at;
        i32 next = at == count ? static_cast<i32>(first) : static_cast<i32>(at) + dir;
        if (next < static_cast<i32>(first)) next = static_cast<i32>(first);
        if (next >= static_cast<i32>(count)) next = static_cast<i32>(count) - 1;
        if (static_cast<u32>(next) == at) return;
        if (l.gesture == kNoGesture && !l.selection) l.default_tracks = true;
        l.edited = false;  // an edited gesture's points are replaced by the new gesture's
        l.gesture = gesture_at(static_cast<u32>(next));
        l.curve = default_curve(l.gesture);  // each gesture starts from its own bend
        // Build 040: picking random or wander rolls new dice (a counter mixed with
        // the lane); any other regeneration keeps the lane's seed.
        if (l.gesture == random_ || l.gesture == wander) l.seed = ++rolls_ * 0x9e3779b1u + lane_ * 0x85ebca6bu;
        // Build 039 (2026-11-23): the fades take any target, starting on the
        // volume while the target follows the gesture (016-038 fixed them to it).
        if (l.gesture == kNoGesture)  // empty again: no tracks, no target (it follows the next gesture)
            l.selection = 0, l.default_tracks = false, l.target = kNoTarget, l.default_target = true;
        commit(store);
    }
    // Build 032 (2026-11-23): hold encoder 3 and press a track key, and
    // the lane is that track alone, its target list its engine's own controls
    // ("track 3" over the target). The same again, or a plain tap of encoder 3,
    // gives the lane back the tracks it had (until the page leaves this
    // variation) and the list goes back to "all": notes, param 1-8 and the
    // fixed targets. Nothing new is saved: a one-track lane is the scope.
    void focus_track(Store& store, u32 t) {
        DraftLane& l = lane();
        const u32 bit = 1u << t;
        if (l.selection == bit) return unfocus(store);
        if (!focus_[lane_].valid) focus_[lane_] = {true, static_cast<u8>(l.selection), l.default_tracks};
        l.selection = bit;
        l.default_tracks = false;
        l.edited = false;  // a gesture's points again, on the one track
        commit(store);
    }
    void unfocus(Store& store) {
        if (!focus_[lane_].valid) return;
        DraftLane& l = lane();
        l.selection = focus_[lane_].selection;
        l.default_tracks = focus_[lane_].default_tracks;
        l.edited = false;
        focus_[lane_].valid = false;
        commit(store);
    }
    // The lane's one track (0-7), or -1 when it has none or several.
    i32 single_track() const {
        const u32 s = lane().selection;
        if (!s || (s & (s - 1))) return -1;
        i32 t = 0;
        while (!(s >> t & 1)) ++t;
        return t;
    }
    // The lane's target, one choice a detent through the list (list_targets()),
    // round at the list's ends. Chosen by hand: it no longer follows the gesture.
    // Build 039: the fades too (016-038 kept them on the volume).
    void choose_target(Store& store, i32 dir) {
        DraftLane& l = lane();
        const u32 count = list_targets();
        u32 at = 0;
        while (at < count && !chosen(choices_[at], l)) ++at;
        const i32 n = static_cast<i32>(count);
        i32 next;
        if (at < count) {
            next = ((static_cast<i32>(at) + dir) % n + n) % n;
        } else if (l.target < opxy::extensions::engine_words) {
            // An engine control the list doesn't offer now (the lane's tracks
            // changed, or a slot past param 4 from older data): on from
            // where the engine's controls would be, before the fixed ones.
            const i32 fixed = n - static_cast<i32>(kFixedTargets);
            next = dir > 0 ? fixed : fixed - 1;
        } else {
            next = dir > 0 ? 0 : n - 1;
        }
        const TargetChoice& c = choices_[next];
        if (chosen(c, l)) return;
        l.target = c.word;
        engine::copy(l.name, c.name, sizeof l.name);
        l.default_target = false;
        commit(store);
    }
    // The SDK's names for the page (Names) and the tracks heard as percussion.
    Engines engines() const {
        return Engines{names_ ? names_->name : nullptr, heard_ ? heard_->percussion : static_cast<u8>(0)};
    }
    // Whether choice c is lane l's target: the same word, and for an engine
    // control the same name (none for a slot).
    static bool chosen(const TargetChoice& c, const DraftLane& l) {
        if (c.word != l.target) return c.name[0] && l.target < opxy::extensions::engine_words && engine::same(c.name, l.name);
        return c.word >= opxy::extensions::engine_words || engine::same(c.name, l.name);
    }
    // choices_: the list for the lane shown (build 025), as above; its length.
    u32 list_targets() {
        u32 n = 0;
        auto add = [&](u32 word, const char* name) {
            if (n >= kMaxChoices) return;
            choices_[n].word = word;
            engine::copy(choices_[n].name, name, kNameCapacity);
            ++n;
        };
        add(kTargetNotes, "");
        const u32 selection = lane().selection;
        if (selection) {
            const Engines e = engines();
            // Build 032: a one-track lane lists its engine's own controls
            // instead of the slots (param 1-8 live in "all"; a track shows
            // just its specific controls), unless its names can't be read.
            const i32 single = single_track();
            const bool slots = single < 0 || !engine::readable(e, static_cast<u32>(single));
            // Build 030: "param 5".."param 8" too (M1's Shift layer), each
            // while some lane track's engine has that slot to move.
            for (u32 w = 0; slots && w < opxy::extensions::engine_words; ++w) {
                bool some = w < kParamSlots;
                for (u32 t = 0; t < kTracks && !some; ++t) {
                    char text[kNameCapacity];
                    some = (selection >> t & 1) && engine::usable(e, t, w, text);
                }
                if (some) add(w, "");
            }
            const u32 names_from = n;
            u32 names = 0;
            // Build 032: an engine's own names only for a one-track lane
            // (encoder 3 + a track key); "all" is the slots and fixed targets.
            const i32 only = single_track();
            for (u32 t = 0; t < kTracks; ++t) {
                if (static_cast<i32>(t) != only) continue;
                for (u32 w = 0; w < opxy::extensions::engine_words && names < kMaxNames; ++w) {
                    char text[kNameCapacity];
                    if (!engine::usable(e, t, w, text)) continue;  // build 034: unnamed ones too, as "param N"
                    bool known = false;
                    for (u32 k = names_from; text[0] && k < n && !known; ++k) known = engine::same(choices_[k].name, text);
                    if (known) continue;
                    add(w, text);
                    ++names;
                }
            }
        }
        for (u32 i = 0; i < opxy::extensions::track_target_count; ++i)
            if (opxy::extensions::track_targets[i].word >= opxy::extensions::engine_words) add(opxy::extensions::track_targets[i].word, "");
        return n;
    }
    // target_text_: the lane's target as the page shows it, its name in full
    // (build 024: no place in the list; build 025: an engine control by slot
    // "param 1", by name its name); target_place_ and target_count_: its
    // place in the list (kept for tests).
    // Build 039 (2026-11-23): a grey "!" after a target that can cost CPU:
    // one that keeps voices sounding longer (the amp envelope's decay, sustain
    // and release; an engine control named length, release, decay or hold). A
    // Drumline length over about 65 % spiked the CPU on the device; the rest is
    // reasoned (voices live longer and overlap), not measured.
    static bool costly(u32 target, const char* name) {
        if (target == 9 || target == 10 || target == 11) return true;
        static const char* const names[] = {"length", "release", "decay", "hold"};
        if (target < opxy::extensions::engine_words && name && name[0])
            for (const char* n : names)
                if (engine::same(name, n)) return true;
        return false;
    }
    void name_target() {
        name_target_text();
        const DraftLane& l = lane();
        if (target_text_[0] && costly(l.target, l.target < opxy::extensions::engine_words ? target_text_ : nullptr)) {
            u32 n = words::length(target_text_);
            words::put(target_text_, n, " !");
        }
    }
    void name_target_text() {
        const DraftLane& l = lane();
        u32 n = 0;
        target_text_[0] = 0;
        target_place_ = 0;
        target_count_ = list_targets();
        for (u32 i = 0; i < target_count_ && !target_place_; ++i)
            if (chosen(choices_[i], l)) target_place_ = i + 1;
        // Build 030: the lane's tracks the target can't move (an engine
        // control the track's engine doesn't have, or may not ramp there):
        // in brackets on screen, their track keys dim (2026-11-23): it
        // needs to be clear when a target won't affect a track. Only where
        // names can be read (in a callback); otherwise every track counts.
        missed_ = 0;
        if (l.target < opxy::extensions::engine_words) {
            const Engines e = engines();
            for (u32 t = 0; t < kTracks; ++t)
                if ((l.selection >> t & 1) && engine::readable(e, t) && engine::word(&e, t, l.target, l.name) < 0)
                    missed_ |= static_cast<u8>(1u << t);
        }
        if (l.target == kNoTarget) return;
        if (l.target == kTargetNotes) return words::put(target_text_, n, "notes");
        const opxy::extensions::TrackTarget* t = opxy::extensions::track_target(l.target);
        if (!t) return;
        if (t->label) return words::put(target_text_, n, t->label);
        if (l.name[0]) return words::put(target_text_, n, l.name);
        words::put(target_text_, n, "param ");
        words::number(target_text_, n, t->position);
    }
    // Encoder 3's value (build 024): the target's name alone, fitted to the
    // strip's room (words::fit).
    void target_value(char* line, u32& n) const {
        if (!target_text_[0]) return words::put(line, n, kNone);
        // Build 052: the strip's 15 px value in 55 px: the long SDK names shortened (words::short_target), then fitted.
        // Build 057 (on 056's " *": "the * doesn't fit, ... it saying none is enough, so let's remove that"): no mark
        // after the name, which takes the strip's full room again. A track the target can't move (missed_) shows only as
        // its dim LED.
        char shown[opxy::draw::text_capacity], name[opxy::draw::text_capacity];
        words::short_target(shown, target_text_);
        words::fit(name, shown, screen::strip_room * screen::unit, screen::value_size);
        words::put(line, n, name);
    }
    // Build 048 (2026-11-23): besides the fixed lengths, lengths that end the
    // window on a later join or the song's end (to the end of scene N), from where
    // it starts, within kMaxLength and the long form's reach (highest()). Only on
    // the start side with the song's shape known. Scene end k: the steps from the
    // join to the end of entry E + k - 1 (k = 1: the section after's end).
    u32 scene_ends(u32* ends, u32* entry_of) const {
        // Not for a window wholly before the join (a break ending on it grows back from the join).
        if (!around_.shape || (split() && !home_is_start()) || draft_.offset + static_cast<i32>(draft_.length) <= 0) return 0;
        const SongShape& s = *around_.shape;
        u32 n = 0, end = 0;
        for (u32 k = 0; k <= kMaxReach && around_.entry + k < s.entries; ++k) {
            end += s.steps[around_.entry + k];
            const i32 length = static_cast<i32>(end) - draft_.offset;
            if (length < 8 || length > static_cast<i32>(kMaxLength)) continue;
            ends[n] = static_cast<u32>(length), entry_of[n] = around_.entry + k, ++n;
        }
        return n;
    }
    // Build 058 (2026-11-24: "I'd like to be able to have another variation span the entire song"): the mirror of
    // scene_ends. Lengths that start the window on an earlier join, keeping its end: for k = 1 up, the summed steps of
    // entries E - k .. E - 1 (start_k) plus the window's end (offset + length), within 8..kMaxLength, to kMaxReach entries
    // and never before entry 0 (no wrap). Without the song's shape none. The split() guard scene_ends has: a song that
    // does not loop and a page on its start (entry 0) has no entry before it, so the loop finds none and the guard says it
    // outright; on its end side (a window before the song's end) there is nothing before entry 0 to wrap to either. Unlike
    // scene_ends a window wholly before the join is fine here (a break may grow back to an earlier join). A start beyond
    // kMaxOffset, or outside lowest()/highest() for its length, is left out, so the end is truly kept.
    u32 scene_starts(u32* starts, u32* entry_of) const {
        if (!around_.shape || split()) return 0;
        const SongShape& s = *around_.shape;
        const i32 end = draft_.offset + static_cast<i32>(draft_.length);
        u32 n = 0, back = 0;
        for (u32 k = 1; k <= kMaxReach && k <= around_.entry && around_.entry - k < s.entries; ++k) {
            back += s.steps[around_.entry - k];
            const i32 length = end + static_cast<i32>(back);
            if (length < 8 || length > static_cast<i32>(kMaxLength) || back > kMaxOffset) continue;
            if (n && starts[n - 1] == static_cast<u32>(length)) continue;
            if (-static_cast<i32>(back) < lowest(static_cast<u32>(length), draft_.offset) || -static_cast<i32>(back) > highest(static_cast<u32>(length), draft_.offset)) continue;
            starts[n] = static_cast<u32>(length), entry_of[n] = around_.entry - k, ++n;
        }
        return n;
    }
    // Build 050 (2026-11-24): the bar lengths a window may take: those the song can hold from its
    // start to the window's end (a fixed length keeps the end; no looping windows), without the one equal to the
    // first scene end (the "until [ ]" entry says it). Build 056 ("I'm no longer seeing the option to resize a
    // variation more than 2 bars"): 050 dropped every length at or above the first scene end; only an equal one
    // duplicates an entry. The first end is what the brief named; a later scene end equal to a fixed length is dropped
    // too (`ends`), else choose_length's lookup of the window's own length would find the fixed entry first and step
    // back along the list. Without the song's shape, all of them.
    bool fixed_fits(u32 length, const u32* ends, u32 n_ends, const u32* starts, u32 n_starts) const {
        for (u32 e = 0; e < n_ends; ++e)
            if (length == ends[e]) return false;
        for (u32 e = 0; e < n_starts; ++e)  // build 058: nor a scene start's
            if (length == starts[e]) return false;
        if (!around_.shape) return true;
        i32 before = 0;
        for (u32 k = 0; k < around_.entry && k < around_.shape->entries; ++k) before += static_cast<i32>(around_.shape->steps[k]);
        return static_cast<i32>(length) <= before + draft_.offset + static_cast<i32>(draft_.length);
    }
    void choose_length(Store& store, i32 dir) {
        // The choices in length order: the bar lengths that fit (keeping the window's end) and the scene
        // ends (keeping its start; build 056: merged by length, were all after the bar lengths), "until end" the last end.
        u32 ends[kMaxReach + 1], entries[kMaxReach + 1];
        const u32 n_ends = scene_ends(ends, entries);
        // Build 058: and the scene starts (keeping the end). One equal to a scene end is left to the end: the column
        // reads "until" for it (length_parts looks there first), so turning to it keeps the start, as it always did.
        u32 starts[kMaxReach], start_entries[kMaxReach];
        u32 n_starts = scene_starts(starts, start_entries);
        for (u32 i = 0; i < n_starts;) {
            bool dup = false;
            for (u32 e = 0; e < n_ends; ++e) dup = dup || ends[e] == starts[i];
            if (!dup) {
                ++i;
                continue;
            }
            for (u32 j = i + 1; j < n_starts; ++j) starts[j - 1] = starts[j], start_entries[j - 1] = start_entries[j];
            --n_starts;
        }
        // Build 063: kLengthChoices + 2 * kMaxReach + 1 = 10 + 190 + 1: every fixed length, up to kMaxReach + 1 scene
        // ends and kMaxReach scene starts, and a spare (unchanged since 058; 8 + 62 + 1 then).
        u32 choice[kLengthChoices + 2 * kMaxReach + 1];
        bool keeps_start[kLengthChoices + 2 * kMaxReach + 1] = {};
        u32 count = 0;
        for (u32 f = 0; f < kLengthChoices; ++f)
            if (fixed_fits(kLengths[f], ends, n_ends, starts, n_starts)) choice[count++] = kLengths[f];
        if (!count) choice[count++] = kLengths[0];  // the shortest always
        // Build 056 ("I'd expect it to expand left to fill the clip even if you're at the end, but right now it
        // blocks the change"): the scene ends join the bar lengths in length order (they came after all of them: a
        // window on "until end" had nothing above it), each at its place; none equals a fixed length (fixed_fits).
        for (u32 e = 0; e < n_ends; ++e) {
            u32 k = count++;
            while (k > 0 && choice[k - 1] > ends[e]) choice[k] = choice[k - 1], keeps_start[k] = keeps_start[k - 1], --k;
            choice[k] = ends[e], keeps_start[k] = true;
        }
        // Build 058: the scene starts, in length order too; they move the start (keeps_start false).
        for (u32 e = 0; e < n_starts; ++e) {
            u32 k = count++;
            while (k > 0 && choice[k - 1] > starts[e]) choice[k] = choice[k - 1], keeps_start[k] = keeps_start[k - 1], --k;
            choice[k] = starts[e], keeps_start[k] = false;
        }
        // Where the window is: its own choice and a step from it, else (a length not listed) the nearest choice in the
        // direction asked: the first longer, or the last shorter.
        u32 at = count;
        for (u32 k = 0; k < count && at == count; ++k)
            if (choice[k] == draft_.length) at = k;
        i32 next;
        if (at < count) {
            next = static_cast<i32>(at) + dir;
        } else if (dir > 0) {
            next = 0;
            while (next + 1 < static_cast<i32>(count) && choice[next] <= draft_.length) ++next;
        } else {
            next = static_cast<i32>(count) - 1;
            while (next > 0 && choice[next] >= draft_.length) --next;
        }
        if (next < 0) next = 0;
        if (next >= static_cast<i32>(count)) next = static_cast<i32>(count) - 1;
        if (choice[next] == draft_.length) return;
        const u32 length = choice[next], old = draft_.length;
        i32 offset = keeps_start[next] ? draft_.offset : draft_.offset + static_cast<i32>(old) - static_cast<i32>(length);  // keeps the end
        if (offset < lowest(length, draft_.offset)) offset = lowest(length, draft_.offset);
        if (offset > highest(length, draft_.offset)) offset = highest(length, draft_.offset);
        for (DraftLane& l : draft_.lanes)
            if (l.gesture == custom || l.edited) rescale(l, line_length(old), line_length(length));  // build 049: a line's own steps
        draft_.length = length;
        draft_.offset = offset;
        commit(store);
    }
    // Build 041: the lane shown's rate one step up or down 1x, 2x, 4x, 8x.
    void choose_rate(Store& store, i32 dir) {
        DraftLane& l = lane();
        const i32 next = static_cast<i32>(rate_bits(l.rate)) + dir;
        if (next < 0 || next > 3 || (1u << next) == l.rate) return;
        l.rate = 1u << next;
        commit(store);
    }
    void choose_amount(Store& store, i32 by) {
        DraftLane& l = lane();
        i32 percent = static_cast<i32>((l.amount * 100 + 127) / 255) + by;
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        const u32 amount = (static_cast<u32>(percent) * 255 + 50) / 100;
        if (amount == l.amount) return;
        l.amount = amount;
        l.edited = false;  // the gesture's points again, by the new amount
        commit(store);
    }
    // A custom lane's line to a new length, in step order, one a step. Build 049: `from` and `to` are the
    // line's lengths (line_length of the window's), so a window longer than 256 steps keeps the line as it is.
    static void rescale(DraftLane& l, u32 from, u32 to) {
        Point* p = l.points;
        u32 kept = 0;
        for (u32 k = 0; k < l.count; ++k) {
            u32 step = p[k].step * to / from;
            if (kept && step <= p[kept - 1].step) step = p[kept - 1].step + 1u;
            if (step >= to) break;
            p[kept] = p[k];
            p[kept++].step = static_cast<u8>(step);
        }
        l.count = static_cast<unsigned char>(kept);
    }

    // The shape page (mockups/variation-points.html): encoder 2's tap
    // from the variation page, on the lane shown's line, the cursor on the
    // window's first step. A lane with no tracks has nothing to move, so the tap does nothing (build 053: the
    // "no tracks" message it used to say is "appeared again... this should have been removed" - the page's own track
    // line, top right, already said "no tracks" (build 056: the strip does now), and 052 moved the message to the bottom row where it read as new).
    // Build 057 ("it's not possible to edit when no tracks are selected, I'm not sure that makes sense"): it opens
    // whatever the lane holds; with no tracks the line and its points draw hidden, and the track keys there choose some.
    void enter_points() {
        points_ = true;
        cursor_ = 0;
        message_ = nullptr;
        ++revision_;
    }
    // One input on the shape page. Encoder 1 moves the cursor by its snap
    // (Shift: the snap), 2 the value at the cursor (a point added there if
    // there is none), 3 the curve into the point at the cursor, 4 jumps to
    // the next or previous point. Encoder 2's tap goes back to the variation
    // page, encoder 1's switches lanes (build 022); Shift + a step key removes
    // the point on that key (build 030; was Shift + M4). Build 071: step keys are the line's points (16 a page), a plain
    // one putting the cursor on that point; - and + the previous and next point (the bank of 16 keys follows the cursor). Track keys toggle the lane's tracks (build 057; 024-056: nothing); M1
    // (or encoder 1's tap) the next lane (a, b, c: build 060), as on the variation page.
    void points_input(Store& store, const PageInput& in) {
        if (in.kind == PageInput::turn && in.step) {
            const i32 dir = in.step > 0 ? 1 : -1;
            if (in.shift) {
                if (in.encoder == 0) {
                    const i32 s = static_cast<i32>(points_snap_) + dir;
                    if (s >= 0 && s < static_cast<i32>(kSnapChoices)) points_snap_ = static_cast<u32>(s), ++revision_;
                }
                return;  // Shift + encoders 2-4 turn nothing here
            }
            switch (in.encoder) {
            case 0: move_cursor(in.step); break;
            case 1: point_value(store, in.step * (in.speed >= 2 ? 5 : 1)); break;
            case 2: point_curve(store, in.step * (in.speed >= 2 ? 4 : 1)); break;
            case 3: {
                const i32 to = edit::next(lane(), cursor_, dir);
                if (to >= 0) cursor_ = static_cast<u32>(to), ++revision_;
                break;
            }
            }
        } else if (in.kind == PageInput::press && in.key >= keys::track && in.key < keys::track + kTracks && !in.shift) {
            // Build 057 ("why don't we allow selecting tracks on the edit page too"): a track key toggles that track
            // in the lane's selection, as on the variation page (without its encoder 3 focus).
            DraftLane& l = lane();
            focus_[lane_].valid = false;
            l.selection ^= 1u << (in.key - keys::track);
            l.default_tracks = false;
            // Build 057: `edited` is left as it is here, unlike the variation page's track key. This page is the
            // hand-drawn line, so a track key must not throw it away; a line never edited by hand still has
            // `edited` false and is regenerated for the tracks as they now are (build/break's stagger).
            commit(store);
        } else if (in.kind == PageInput::press && in.key == keys::m1 + 1 && !in.shift) {
            // Build 056: "mute" over M2 on the shape page too, the lane shown's (as the variation page's).
            DraftLane& l = lane();
            l.muted = !l.muted;
            commit(store);
        } else if (in.kind == PageInput::press && in.key == keys::m1 + 2 && !in.shift) {
            points_ = false;  // build 054: M3 (it was encoder 2's tap, the way in); build 055: marked "back"
            name_target();
            ++revision_;
        } else if (in.kind == PageInput::press && in.key >= keys::step && in.key < keys::step + 16 && in.shift) {
            // Build 030: Shift + a step key removes a point. Build 071 ("the steps could reflect the points on the
            // shape"): the key is the page's point, not a step; a key with no point does nothing.
            const u32 idx = points_page() * 16 + (in.key - keys::step);
            if (idx >= lane().count) return;
            const u32 to = lane().points[idx].step;
            if (to != cursor_) cursor_ = to, ++revision_;
            remove_point(store);
        } else if (in.kind == PageInput::click && in.encoder == 3 && in.shift) {
            // Build 064 (on the Shift layer's "step key" / "remove" over encoder 4's column: "yes shift & encoder 4
            // click makes sense"): the label points at encoder 4, so its click removes the point at the cursor, as
            // Shift + a step key does. Shift + encoders 2-4 turn nothing on this page, so the click is unambiguous.
            remove_point(store);
        } else if (in.kind == PageInput::press && in.key == keys::m1) {
            switch_lane(store);  // build 060: a, b, c, plain or with Shift, as the variation page's M1 (057)
        } else if (in.kind == PageInput::press && in.key >= keys::step && in.key < keys::step + 16) {
            // Build 071: a step key moves the cursor to that point; none behind it, nothing.
            const u32 idx = points_page() * 16 + (in.key - keys::step);
            if (idx >= lane().count) return;
            const u32 to = lane().points[idx].step;
            if (to != cursor_) cursor_ = to, ++revision_;
        } else if (in.kind == PageInput::press && (in.key == keys::minus || in.key == keys::plus)) {
            // Build 071 ("+/- on the edit page aren't working correctly, actually, when there's two very close points,
            // one is skipped"): - and + went a fixed 16 steps, landing only on multiples of 16 and passing points off that
            // grid. They now step to the previous / next point, the search encoder 4 uses (edit::next), skipping none.
            // The step-key bank follows the cursor (points_page), so there is no paging by hand.
            const i32 to = edit::next(lane(), cursor_, in.key == keys::plus ? 1 : -1);
            if (to >= 0) cursor_ = static_cast<u32>(to), ++revision_;
        }
    }
    // The cursor by the snap: to its grid first, then by whole snaps, within the window.
    void move_cursor(i32 detents) {
        const i32 snap = static_cast<i32>(kSnaps[points_snap_]), c = static_cast<i32>(cursor_);
        const i32 base = c / snap * snap;
        i32 to = detents > 0 ? base + detents * snap : (c == base ? base : base + snap) + detents * snap;
        if (to < 0) to = 0;
        if (to > static_cast<i32>(line_length(draft_.length)) - 1) to = static_cast<i32>(line_length(draft_.length)) - 1;
        if (to == c) return;
        cursor_ = static_cast<u32>(to);
        ++revision_;
    }
    // An edit of the lane's line: a gesture's lane becomes custom
    // (edited: its gesture kept, so the variation page's gesture, amount
    // and tracks regenerate from it), its tracks fixed; into the store.
    void points_edited(Store& store) {
        DraftLane& l = lane();
        if (l.gesture != custom && l.gesture != kNoGesture) l.edited = true;
        l.default_tracks = false;
        commit(store);
    }
    // Encoder 2: the value at the cursor `by` percent on, a point added there
    // first if there is none (where the line is).
    void point_value(Store& store, i32 by) {
        DraftLane& l = lane();
        if (!l.selection) return;  // a lane with no tracks: nothing it moves
        i32 k = edit::find(l, cursor_);
        const bool added = k < 0;
        if (added) k = edit::add(l, cursor_);
        if (k < 0) return say("points full");
        Point& p = l.points[k];
        const u32 v = edit::nudge(p.value, by);
        if (v == p.value && !added) return;
        p.value = static_cast<u8>(v);
        points_edited(store);
    }
    // Encoder 3: the curve of the line into the point at the cursor (none
    // into the line's first point: nothing runs into it).
    void point_curve(Store& store, i32 detents) {
        DraftLane& l = lane();
        if (!l.selection) return;  // a lane with no tracks: nothing it moves
        const i32 k = edit::find(l, cursor_);
        if (k < 0) return say("no point");
        if (k == 0) return;
        Point& p = l.points[k];
        const i32 curve = edit::curve_step(p.curve, detents);
        if (curve == p.curve) return;
        p.curve = static_cast<signed char>(curve);
        points_edited(store);
    }
    // Shift + a step key: the point at the cursor goes (the last one
    // too: the line then holds as set).
    void remove_point(Store& store) {
        if (!lane().selection) return;
        if (!edit::remove(lane(), cursor_)) return say("no point");
        points_edited(store);
    }

    // Build 024: a window may lie wholly in the section before the join. One
    // that ends on the join is still a transition over it (default tracks:
    // those arriving or leaving); one that ends before it is inside the
    // section before, whose tracks it takes as a section's own.
    bool before_join() const { return draft_.offset + static_cast<i32>(draft_.length) < 0; }
    bool over_join() const { return draft_.offset < 0 && !before_join(); }
    u32 heard(u32 scene) const { return heard_ && scene < kScenes ? heard_->scene[scene] : 0; }
    // The draft's points again, then into the store.
    void commit(Store& store) {
        ++revision_;
        fresh_ = false;
        draft_.scene_before = around_.scene_before < kSceneCount ? around_.scene_before : kNoScene;
        draft_.scene_after = around_.scene_after < kSceneCount ? around_.scene_after : kNoScene;
        draft_.unplaced = false;
        // Build 047: the later joins the window crosses, from the song's sections as they are.
        draft_.reach = 0;
        if (around_.shape) {
            const SongShape& s = *around_.shape;
            draft_.reach = reach_of(s.scene, s.steps, s.entries, around_.entry, draft_.offset, draft_.length, draft_.reach_scene, draft_.reach_start);
        }
        for (u32 j = draft_.reach; j < kMaxReach; ++j) draft_.reach_scene[j] = kNoScene, draft_.reach_start[j] = 0;
        // Build 058: the backward list made again too (a moved window would else fail put's validation); the page's
        // own view of it is stage B.
        draft_.back = 0;
        if (around_.shape) {
            const SongShape& s = *around_.shape;
            draft_.back = back_reach_of(s.scene, s.steps, s.entries, around_.entry, draft_.offset, draft_.length, draft_.back_scene, draft_.back_start);
        }
        for (u32 j = draft_.back; j < kMaxReach; ++j) draft_.back_scene[j] = kNoScene, draft_.back_start[j] = 0;
        // A window wholly before the join takes the section before's tracks as a section's own.
        const u32 before = heard(around_.scene_before), after = before_join() ? before : heard(around_.scene_after);
        for (DraftLane& l : draft_.lanes) {
            if (l.gesture == custom || l.edited) continue;  // their own points, as edited
            if (l.default_tracks && l.gesture != kNoGesture)
                l.selection = default_tracks(l.gesture, over_join(), before, after, heard_ ? heard_->percussion : 0);
            if (l.default_target) l.target = natural_target(l.gesture);  // build 039: a fade's too, by choice
            generate(l, draft_.length);
        }
        name_target();
        if (store.put(index_, draft_)) {
            stored_ = true;
        } else {
            stored_ = false;
            index_ = store.count();
            say("store full");  // not kept
        }
        recount(store);
    }
    u8 missed_ = 0;  // the lane's tracks its target can't move (name_target)
    void say(const char* text) {
        u32 n = 0;
        message_text_[0] = 0;
        while (*text && n + 1 < sizeof message_text_) message_text_[n++] = *text++;
        message_text_[n] = 0;
        message_ = message_text_;
        message_left_ = 3 * screen::unit / 2;  // 1.5 s, Q10
        ++revision_;
    }

    // Build 049 (2026-11-23): the variation page's timeline. Its view is [from, to) in steps from the join: the
    // smallest of 32, 64 ... 2048 steps holding twice the window (the window a half of the plot at most), capped by
    // the song the window may occupy; kept still while the window moves inside it (a window reaching its edge
    // shifts it just enough to touch), placed again when a variation is shown or the span changes: centred on the
    // join when the window's middle is within half the span of it, else on the window, then on a 4 step grid and
    // inside the song. The drawn view eases to that target (ease_view).
    //
    // The song the window may occupy: from where it can start to where the furthest start ends it (the window's
    // own place too, a window not on its split home side is not stuck: build 032).
    void extent(i32* lo, i32* hi) const {
        const i32 length = static_cast<i32>(draft_.length);
        *lo = lowest(draft_.length, draft_.offset);
        *hi = highest(draft_.length, draft_.offset) + length;
        if (draft_.offset < *lo) *lo = draft_.offset;
        if (draft_.offset + length > *hi) *hi = draft_.offset + length;
        // Build 069 ("end isn't there, despite me moving as far to the right as I can"). The extent was built
        // only from where the *window* may sit, never from where the song actually ends. `start_cap` holds the
        // window's start one step inside its own section, so on a join whose section is shorter than the window the
        // view's right edge stopped one step short of the song's end line - and a line outside the view is not drawn,
        // so "end" went missing exactly when he had moved as far that way as he could. The song's own start and end
        // are part of the extent now. It only ever loosens a bound: the span still comes from the window's length and
        // the view is still placed around the window, so nothing moves except that the song's edges can be reached.
        if (around_.shape) {
            const i32 start = song_start_at(), end = song_end();
            if (start < 0 && start < *lo) *lo = start;
            if (end > 0 && end > *hi) *hi = end;
        }
    }
    void aim_view() {
        using namespace screen;
        const i32 length = static_cast<i32>(draft_.length), offset = draft_.offset, end = offset + length;
        i32 lo, hi;
        extent(&lo, &hi);
        i32 span = span_min;
        while (span < span_max && span < 2 * length) span *= 2;
        if (span > hi - lo) span = hi - lo;
        i32 from = view_from_;
        if (!view_set_ || view_to_ - view_from_ != span) {
            const i32 twice_middle = 2 * offset + length;  // twice the window's middle from the join
            from = (twice_middle < span && twice_middle > -span) ? -span / 2 : offset + length / 2 - span / 2;
            const i32 rounded = from + 2;  // the nearest 4 steps (the mockup's Math.round)
            from = rounded - ((rounded % 4) + 4) % 4;
        }
        if (offset < from) from = offset;  // the window left the view: just enough to touch its edge
        if (end > from + span) from = end - span;
        if (from < lo) from = lo;
        if (from + span > hi) from = hi - span;
        // Build 073 ("changing the variation to a bar in length restores the end ... but if I move the variation
        // so it scrolls to the left and then come back, it's gone back to it's buggy state"). That experiment located
        // it. A length change alters the span and the view is placed anew; a move keeps the old `from` and only nudges
        // it enough to hold the window - `from = end - span` - so the view's right edge came to rest exactly on the
        // window's end. Everything past it, the rest of the bar and the song's end line, fell outside the drawn view,
        // which is why "end" went missing for a half-bar window but not for a bar-long one, whose end *is* the song's.
        // Nothing ever pulled the view back the other way. So: when the extent's end is within a screen of the
        // window's end, the view's right edge goes to the extent's end. The window stays inside it, because the
        // condition is exactly that it would.
        // ...but only once the window is at its bound, which is the case reported ("move the variation so it
        // scrolls to the left and then come back"). Pulling the view right whenever the song's end was near
        // shifted it under a window that had not moved there, and pushed the playhead out of view during
        // playback - `tests/extension_page.cpp` caught exactly that.
        // ...and only once the window can move no further right under the snap. "At the raw bound" was too
        // strict: build 068 stops a move on the snap's grid, so the window rests short of it (240 where the
        // bound is 248). Pulling the view whenever the song's end was merely near shifted it under a window
        // that had not moved there and pushed the playhead out during playback, which
        // `tests/extension_page.cpp` caught.
        const i32 hi_off = highest(draft_.length, offset), move_snap = static_cast<i32>(kSnaps[snap_]);
        const i32 grid_hi = (hi_off >= 0 ? hi_off / move_snap : -((-hi_off + move_snap - 1) / move_snap)) * move_snap;
        if (offset >= grid_hi && hi - end <= span && from + span < hi) from = hi - span;
        view_from_ = from, view_to_ = from + span, view_set_ = true;
    }
    // The drawn view toward the target: k = ms * ease_rate / 1000 of the way each frame (elapsed is Q10 s), the
    // distance snapped away within half a pixel; at once the first frame after open or a new join (view_snap_).
    // True while it is still moving (the page then asks for the next frame).
    bool ease_view(i32 elapsed, bool snap) {
        const i32 aim_from = view_from_ * screen::view_q, aim_to = view_to_ * screen::view_q;
        if (snap || view_snap_ || !view_eased_) {
            view_snap_ = false, view_eased_ = true;
            drawn_from_ = aim_from, drawn_to_ = aim_to;
            return false;
        }
        i32 k = (elapsed > 0 ? elapsed : 0) * screen::ease_rate;
        if (k > screen::unit) k = screen::unit;
        const i32 df = aim_from - drawn_from_, dt = aim_to - drawn_to_;
        const i32 span = aim_to - aim_from;  // Q6 steps across the plot: half a pixel is span / 864 of them
        auto within_half_px = [&](i32 d) { return (d < 0 ? -d : d) * (2 * (screen::plot_x1 - screen::plot_x0)) < span; };
        if (within_half_px(df) && within_half_px(dt)) {
            drawn_from_ = aim_from, drawn_to_ = aim_to;
            return false;
        }
        auto step = [&](i32 d) {
            const i32 s = d * k / screen::unit;
            return s ? s : d > 0 ? 1 : d < 0 ? -1 : 0;  // always on its way
        };
        drawn_from_ += step(df), drawn_to_ += step(dt);
        return drawn_from_ != aim_from || drawn_to_ != aim_to;
    }
    // Where step `q` (Q6, inside the drawn view) is across the plot, Q10 pixels; whole eighths of a pixel (the
    // products inside 32 bits at the widest view).
    i32 view_x(i32 q) const {
        using namespace screen;
        i32 span = drawn_to_ - drawn_from_;
        if (span < 1) span = 1;
        if (q < drawn_from_) q = drawn_from_;
        if (q > drawn_to_) q = drawn_to_;
        return plot_x0 * unit + (q - drawn_from_) * ((plot_x1 - plot_x0) * 8) / span * (unit / 8);
    }
    // The song's sections from the one before the join on, with where each starts and ends (steps from the join) and
    // its scene (0-based; kNoScene unknown): the one before, the one after, then those after that. From the song's
    // shape when the page has it; else the window's reach list (the last of those has no known end: it runs to
    // `unknown_end`). `fn` is called with each in turn until it returns false.
    struct Section { i32 start, end; u32 scene; };
    // Build 051: the song's end, in steps from the join, when the page has the song's shape; else -1.
    i32 song_end() const {
        if (!around_.shape) return -1;
        i32 end = 0;
        for (u32 e = around_.entry; e < around_.shape->entries; ++e) end += static_cast<i32>(around_.shape->steps[e]);
        return end;
    }
    // Build 061 ("if you're in a variation that isn't at the start of the song, you don't see start on the
    // timeline when moving the variation, which is confusing"): the song's start, steps from this join and so
    // negative, the mirror of song_end(). Build 058 made this reachable - a window can now run back across several
    // earlier scenes - and the end line has been named since 051 while the start line never was. 0 at the join
    // itself (entry 0), where the join's own line already marks it.
    i32 song_start_at() const {
        if (!around_.shape) return 1;  // not known: nothing to draw
        i32 back = 0;
        for (u32 e = 0; e < around_.entry && e < around_.shape->entries; ++e) back += static_cast<i32>(around_.shape->steps[e]);
        return -back;
    }
    template <class Fn> void each_section(Fn& fn, i32 unknown_end) const {
        Section sec = {-static_cast<i32>(around_.steps_before), 0, around_.scene_before};
        if (!fn(sec)) return;
        sec = {0, static_cast<i32>(around_.steps_after), around_.scene_after};
        if (!fn(sec)) return;
        if (around_.shape) {
            const SongShape& s = *around_.shape;
            i32 at = static_cast<i32>(around_.steps_after);
            for (u32 k = 1; k <= kMaxReach && around_.entry + k < s.entries; ++k) {
                const u32 e = around_.entry + k;
                sec = {at, at + static_cast<i32>(s.steps[e]), s.scene[e]};
                if (!fn(sec)) return;
                at = sec.end;
            }
            return;
        }
        for (u32 j = 0; j < draft_.reach; ++j) {
            sec = {static_cast<i32>(draft_.reach_start[j]),
                   j + 1 < draft_.reach ? static_cast<i32>(draft_.reach_start[j + 1]) : unknown_end, draft_.reach_scene[j]};
            if (!fn(sec)) return;
        }
    }
    // The playhead's place in whole pixels, or -1 when it isn't shown. On the timeline the song's place in the drawn
    // view across the plot (the playhead's own ticks a step: kStepTicks).
    i32 playhead_x() const {
        if (!around_.song || !heading_) return -1;
        if (points_) {
            // The plot's x of the song's place in the window (step 0 at plot_x0, the window's end at plot_x1).
            const i32 tick = static_cast<i32>(kStepTicks), into = playhead_ - draft_.offset * tick;
            const i32 last = (static_cast<i32>(draft_.length) - 1) * tick, end = static_cast<i32>(draft_.length) * tick;
            if (into < 0 || into > last) return -1;
            return screen::plot_x0 + into / 16 * (screen::plot_x1 - screen::plot_x0) / (end / 16);
        }
        const i32 q = playhead_ * screen::view_q / static_cast<i32>(kStepTicks);
        if (q < drawn_from_ || q > drawn_to_ || drawn_to_ <= drawn_from_) return -1;
        return screen::plot_x0 + (q - drawn_from_) * (screen::plot_x1 - screen::plot_x0) / (drawn_to_ - drawn_from_);
    }
    bool playhead_inside() const {
        const i32 tick = static_cast<i32>(kStepTicks);
        return playhead_ >= draft_.offset * tick && playhead_ < (draft_.offset + static_cast<i32>(draft_.length)) * tick;
    }

    // The title (build 022): the page's name, the variation's number in the
    // song spelled out ("variation 2 of 5"), new until its first edit.
    void title(char* line) const {
        u32 n = 0;
        line[0] = 0;
        words::put(line, n, "variation");
        if (number_) {
            words::put(line, n, " ");
            words::number(line, n, number_);
            words::put(line, n, " of ");
            words::number(line, n, total_);
        }
        if (fresh_ && n + 4 <= screen::text_chars) words::put(line, n, " new");
    }
    // The join into entry 1: the song's start (its end before it, looping).
    bool song_start() const { return around_.entry == 0; }
    // Build 047: the last scene a window reaches across later joins (0-based), or kNoScene.
    u32 last_reached() const { return draft_.reach ? draft_.reach_scene[draft_.reach - 1] : kNoScene; }
    template <class Put, class Number> void reach_tail(Put& put, Number& number) const {
        if (last_reached() >= kSceneCount) return;
        put(" to ");
        number(last_reached() + 1);
    }
    // The join's scenes (only the one after when the window is inside it), as
    // much as fits in `room` Q10 pixels: "scene 1 > scene 2", else "scene 1 >
    // 2", else none (the ruler names them too).
    void join(char* line, i32 room) const {
        char full[48];
        for (u32 form = 0; form < 3; ++form) {
            u32 n = 0;
            auto put = [&](const char* t) { while (*t && n + 1 < sizeof full) full[n++] = *t++; };
            auto number = [&](u32 v) {
                char digits[10];
                u32 count = 0;
                do { digits[count++] = static_cast<char>('0' + v % 10); v /= 10; } while (v);
                while (count && n + 1 < sizeof full) full[n++] = digits[--count];
            };
            if (form < 2 && song_start()) {
                // Build 024: the join into entry 1 is the song's start (and,
                // looping, its end): "start > scene 1" for a window at or
                // after it, "scene 2 > end" for one before it (the song's
                // last section, before it loops), "end > start" for one
                // across it (023 said "scene 2 > scene 1").
                // Build 025: a song that doesn't loop shows only the side
                // that exists, the start or the end (split()).
                const i32 end = draft_.offset + static_cast<i32>(draft_.length);
                if (draft_.offset >= 0 || (split() && end > 0)) {  // build 030: across the start, the start side
                    put(form == 0 ? "start > scene " : "start > ");
                    number(around_.scene_after + 1);
                    reach_tail(put, number);
                } else if (end <= 0 || split()) {
                    put(form == 0 ? "scene " : "");
                    number(around_.scene_before + 1);
                    put(" > end");
                } else {
                    put("end > start");
                }
            } else if (form < 2 && last_reached() < kSceneCount) {
                // Build 047: a window that runs on across later joins: "scene 1 > 2 to 4" (then "1 > 2 to 4"),
                // the last scene it reaches at the end.
                if (form == 0) put("scene ");
                if (draft_.offset < 0) {
                    number(around_.scene_before + 1);
                    put(" > ");
                }
                number(around_.scene_after + 1);
                reach_tail(put, number);
            } else if (form < 2) {
                put("scene ");
                if (draft_.offset < 0) {
                    number(around_.scene_before + 1);
                    put(form == 0 ? " > scene " : " > ");
                }
                number(around_.scene_after + 1);
            }
            full[n] = 0;
            if (n <= screen::text_chars && words::width(full, screen::join_size) <= room) break;
        }
        u32 n = 0;
        line[0] = 0;
        words::put(line, n, full);
    }
    // The window's length: "2 beats", "1 bar", "2 bars".
    static const char* rate_text(u32 rate) { return rate == 2 ? "2x" : rate == 4 ? "4x" : rate == 8 ? "8x" : "1x"; }
    // Build 052 (2026-11-24): the strip's column 2 in the Shift layer, the unit in the name and the number in the
    // value (today's single string, e.g. "until [25], 8", did not fit 55 px at 15 px): "until" / "end", "until" /
    // "[25], 8", "beats" / "6", "bar" / "1", "bars" / "2".
    void length_parts(char* name, char* value) const {
        u32 nn = 0, vn = 0;
        name[0] = value[0] = 0;
        // Build 048: a length that isn't a fixed one and ends on a later join or the
        // song's end names it. Build 049 (2026-11-23): by the song page's own
        // grid (columns 1-8 along the top, each row's first entry down the side), as
        // a scene can play more than once: "until [9], 3" (the row starting 9, column
        // 3: entry 11), "until end" (048 said "to scene 2", which read as its start
        // and named no one repeat).
        // Build 050: a scene end names itself even when it is a bar length too. Build 056: the bar lengths are no
        // longer capped below the first scene end, so choose_length drops any that equals an end and the two
        // never name the same length twice.
        {
            u32 ends[kMaxReach + 1], entries[kMaxReach + 1];
            const u32 n_ends = scene_ends(ends, entries);
            for (u32 k = 0; k < n_ends; ++k) {
                if (ends[k] != draft_.length) continue;
                words::put(name, nn, "until");
                if (entries[k] + 1 == around_.shape->entries) return words::put(value, vn, "end");
                words::put(value, vn, "[");
                words::number(value, vn, entries[k] / 8 * 8 + 1);
                words::put(value, vn, "], ");
                return words::number(value, vn, entries[k] % 8 + 1);
            }
        }
        // Build 058: a length that starts the window on an earlier join (keeping its end) reads "from" the join's place on the
        // song page's grid, or "from" / "start" where it is the song's start (entry 0). After "until": that is looked up first.
        {
            u32 starts[kMaxReach], entries[kMaxReach];
            const u32 n_starts = scene_starts(starts, entries);
            for (u32 k = 0; k < n_starts; ++k) {
                if (starts[k] != draft_.length) continue;
                words::put(name, nn, "from");
                if (entries[k] == 0) return words::put(value, vn, "start");
                words::put(value, vn, "[");
                words::number(value, vn, entries[k] / 8 * 8 + 1);
                words::put(value, vn, "], ");
                return words::number(value, vn, entries[k] % 8 + 1);
            }
        }
        if (draft_.length % 16) {
            words::put(name, nn, "beats");
            words::number(value, vn, draft_.length / 4);
        } else {
            words::put(name, nn, draft_.length == 16 ? "bar" : "bars");
            words::number(value, vn, draft_.length / 16);
        }
    }
    template <class F> void draw_playhead(F& frame);
    template <class F> void draw_timeline(const PageTick& tick, F& frame);
    template <class F> void draw_curves(F& frame);
    template <class F> void draw_scene_names(F& frame);
    template <class F> void draw_bar_numbers(F& frame);
    template <class F> void place_as_set(F& frame);
    template <class F> void draw_letters(F& frame, i32 x, i32 y, i32 scale);
    template <class F> i32 header_row(F& frame, char* line);
    template <class F> void draw_texts(F& frame);
    template <class F> void draw_points(F& frame);
    template <class F> void draw_points_texts(F& frame);

    Around around_ = {};
    u32 opened_entry_ = 0;  // the join the page opened on
    const Heard* heard_ = nullptr;
    const Names* names_ = nullptr;
    Draft draft_ = {};
    u32 lane_ = 0;          // the lane shown: 0 A, 1 B, 2 C (build 060)
    char target_text_[opxy::draw::text_capacity] = {};  // the target's name
    TargetChoice choices_[kMaxChoices] = {};  // encoder 3's list for the lane shown (list_targets)
    u32 target_place_ = 0, target_count_ = 0;
    u32 index_ = 0, number_ = 0, total_ = 0, generation_ = 0, snap_ = 2;
    bool stored_ = false, fresh_ = false;
    u32 rolls_ = 0;  // build 040: dice rolled so far, for random and wander's seeds
    bool followed_ = false;  // lane B has had its chance to take lane A's gesture (lane C never takes one: build 060)
    // Each lane's target before a fade took it over (the fades are always on the
    // volume): given back when the gesture turns on past the fades.
    struct Kept {
        bool valid;
        u32 target;
        bool follows;
    };
    Kept kept_[kLanes] = {};
    // Build 032: the lane's tracks before encoder 3 + a track key narrowed it
    // to one (valid: there is one to give back), per lane; and whether a
    // track key went with encoder 3's hold (its click then does nothing).
    struct Focus {
        bool valid;
        u8 selection;
        bool default_tracks;
    };
    Focus focus_[kLanes] = {};
    bool focused_with_hold_ = false;
    bool shift_ = false;    // Shift held: the bottom row shows the Shift layer
    bool points_ = false;   // the shape page shows
    u32 cursor_ = 0;        // its cursor, a step of the line (the window's, up to 256: build 049; a longer window's line is stretched over it)
    u32 points_snap_ = 0;   // its cursor's snap: step first
    const char* notice_ = nullptr;  // said over the song page as the page closes
    bool heading_ = false;  // the song plays within the join's two sections
    i32 playhead_ = 0;      // there, in ticks from the join
    i32 shown_x_ = -1;      // the playhead as last drawn (-1 hidden)
    const char* message_ = nullptr;
    char message_text_[24] = {};
    i32 message_left_ = 0;
    u32 revision_ = 1, drawn_ = 0;
    // Build 049: the timeline's view. The target (view_from_, view_to_: steps from the join) is aimed on each
    // redraw (aim_view); the drawn one (Q6 steps) eases to it. view_set_ false: place the view anew; view_snap_:
    // draw it at the target (the first frame after open, a new join).
    i32 view_from_ = 0, view_to_ = 0, drawn_from_ = 0, drawn_to_ = 0;
    bool view_set_ = false, view_snap_ = true, view_eased_ = false;
};

// A lane's overall motion, 0..255 at `at` (scene clock units into the
// window): the mean of its tracks' values (each following the lane's line,
// build's and break's one after another: stagger.hpp; no points: as set),
// at its true height (build 075: the same as the edit page's): 0 the bottom, 255 the top, as set the
// middle, so a gesture that only works below as set uses only the lower half. A lane that
// doesn't move (an amount of 0, a line all at as set) is a flat line across the middle; a flat line elsewhere (build 064: hold at
// full amount, all 0) draws at its own value.
const u32 kMotionSamples = screen::points / 2;
inline u32 lane_mean(const DraftLane& l, u32 length, u32 at) {
    u32 sum = 0, n = 0;
    // Build 046: the line as playback reads it (window.hpp targets()): at the
    // lane's rate, the gate's one stored bar repeated every bar.
    // Build 047: stutter's line is its density, read once over the window (its rate is the
    // retrigger's top speed, not a repeat of the line): rate 1.
    const u32 rate = l.gesture == kGestureGate ? l.rate * stutter_scale(length) : is_stutter(l.gesture) ? 1 : l.rate;
    for (u32 t = 0; t < kTracks; ++t) {
        if (!(l.selection >> t & 1)) continue;
        const u32 shift = stagger(l.gesture, l.amount, l.curve, l.selection, t, length);
        sum += l.count ? value_late(l.points, l.count, at, shift, rate, length) >> 8 : kAsSet;
        ++n;
    }
    return n ? sum / n : kAsSet;
}
inline void motion(const DraftLane& l, u32 length, u32* out) {
    const u32 span = length * kStepTicks - 1;
    u32 low = 255, high = 0;
    // Build 048 (2026-11-23): a line repeating faster than the samples can
    // follow (the gate at 8x: a cycle every step) aliased: at 8x every sample
    // fell on the same phase and the gate drew as one hump. Under 4 samples a
    // cycle, each sample takes its cell's highest or lowest value in turn, a
    // zigzag between the line's extremes: a fast chop drawn as one.
    const u32 cycles = l.gesture == kGestureGate ? length * l.rate / 8 : is_stutter(l.gesture) ? 1 : l.rate;
    const bool band = cycles * 4 > kMotionSamples;
    for (u32 i = 0; i < kMotionSamples; ++i) {
        const u32 at = span * i / (kMotionSamples - 1);
        if (band) {
            const u32 cell = span / (kMotionSamples - 1), from = at > cell / 2 ? at - cell / 2 : 0;
            u32 lo = 255, hi = 0;
            for (u32 k = 0; k < 8; ++k) {
                u32 t = from + cell * k / 7;
                if (t > span) t = span;
                const u32 m = lane_mean(l, length, t);
                if (m < lo) lo = m;
                if (m > hi) hi = m;
            }
            out[i] = i % 2 ? lo : hi;
        } else {
            out[i] = lane_mean(l, length, at);
        }
        if (out[i] < low) low = out[i];
        if (out[i] > high) high = out[i];
    }
    for (u32 i = 0; i < kMotionSamples; ++i) {
        u32 v = out[i];
        // Build 075 (on the fade+ at 100% photographed on both screens: "these photos show the same variation,
        // one on the variation screen and the other on the edit screen, but the curve on the edit screen only goes up
        // 50% of the grid"): no normalising. The doubling that stretched a one-sided gesture to fill the chart is gone,
        // so every sample draws at its true height, as the edit page's draw_points does: the bottom half below the
        // track's set level, the top half above it. Only a line all at as set draws across the middle (that is also its
        // true height, kAsSet; kept named in case a later build changes the scale).
        if (low == high && low == kAsSet) v = kAsSet;
        out[i] = v > 255 ? 255 : v;
    }
}
// Whether a lane has anything to draw: a gesture and points.
inline bool lane_moves(const DraftLane& l) {
    return l.gesture != kNoGesture && l.selection && l.count;
}

// A thin ribbon through `top` (half the outline's points, left to right):
// out along them, back 1.5 px beneath (at the node's scale 2), so it encloses
// area wherever it runs (docs/display.md, drawing on the device).
template <class Path> void ribbon(Path& path, const opxy::draw::Point* top, u32 seed) {
    using namespace screen;
    namespace draw = opxy::draw;
    const u32 half = points / 2;
    const i32 thick = 3 * unit / 4;  // 1.5 px at the node's scale 2
    for (u32 i = 0; i < half; ++i) {
        const draw::Point a = top[i > 0 ? i - 1 : 0], b = top[i + 1 < half ? i + 1 : half - 1];
        const i32 dx = (b.x - a.x) / 32, dy = (b.y - a.y) / 32;
        const i32 len = static_cast<i32>(draw::square_root(static_cast<u32>(dx * dx + dy * dy)));
        // The normal below the line: (-dy, dx), unit length.
        const i32 nx = len ? -dy * thick / len : 0, ny = len ? dx * thick / len : thick;
        path.points[i] = top[i];
        path.points[points - 1 - i].x = top[i].x + nx;
        path.points[points - 1 - i].y = top[i].y + ny;
    }
    u32 key = 2166136261u + seed;
    for (u32 i = 0; i < points; ++i)
        key = (key ^ static_cast<u32>(path.points[i].x)) * 16777619u ^ static_cast<u32>(path.points[i].y);
    path.key = key ? key : 1;
}

// A path laid flat as the artwork draws it: 32 points along the top, 32 back 0.75 below, all distinct (a node that
// shows nothing still carries a valid path).
template <class Path> void flat_ribbon(Path& path) {
    using namespace screen;
    for (u32 i = 0; i < points / 2; ++i) {
        const i32 x = (-60 + 120 * static_cast<i32>(i) / static_cast<i32>(points / 2 - 1)) * unit;
        path.points[i] = {x, 0};
        path.points[points - 1 - i] = {x, 3 * unit / 4};
    }
    path.key = 1;
}

// A value up the plot (0 at plot_y1, 255 at plot_y0), Q10 pixels.
inline i32 plot_up(u32 value, i32 dy = 0) {
    using namespace screen;
    return (plot_y1 + dy) * unit - static_cast<i32>(value) * (plot_y1 - plot_y0) * unit / 255;
}

// The shape page's dashed as set line (build 023), now the variation page's too (build 049): across the plot
// at as set, dim.
template <class F> void VariationPage::place_as_set(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    const i32 dy = points_ ? 0 : tl_dy;  // build 050: the timeline's place
    for (u32 i = 0; i < as_set_count; ++i)
        draw::place(frame, n_as_set + i, (plot_x0 + as_set_pitch * static_cast<i32>(i)) * unit,
                    (plot_y1 + dy) * unit - static_cast<i32>((kAsSet << 8) * static_cast<u32>(plot_y1 - plot_y0) / 255) * unit / 256, unit, s_dim,
                    draw::fill_and_stroke);
}

// Build 049 (2026-11-23): the lanes' curves on the timeline, each a ribbon of motion()'s 32 samples
// across the window's place in the drawn view, clamped to the plot (a window part way out of the view, while it
// eases, is cut at the plot's edge: the samples read between; the one in view is exact). The other lanes dim behind,
// the lane shown bright (a muted lane dim: build 047).
//
// Build 060 (stage B: a lane that vanishes as you zoom "will read as buggy", so one rule whatever the zoom or
// content): the page has 4 mutable outlines of 64 points, and a lane wider than a path's reach takes two, so only the lane
// being edited can be drawn from paths. It is drawn from them exactly as before (smooth, bright). The two others are drawn
// from nodes, a coarser sketch in the same dim shade: sketch_pieces thin ribbons, each laid from one motion sample's place
// to the next (draw::segment). Their paths (n_other, n_other_b) are hidden here. An empty lane draws nothing, as before.
//
// A path reaches 64 px either side of its node, at most 256 px across at scale 2, so a curve wider than that (a
// window over more than 59 % of the view: a song too short to hold twice the window, or the view mid ease) is
// the lane shown across two paths (build 050). One narrower than curve_min px is left out (the path's points
// would run together; the sketch pieces are left out with it, for the same reason: they would be dots).
template <class F> void VariationPage::draw_curves(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    const i32 a = draft_.offset * view_q, b = a + static_cast<i32>(draft_.length) * view_q;
    const i32 va = a > drawn_from_ ? a : drawn_from_, vb = b < drawn_to_ ? b : drawn_to_;
    const i32 width = vb > va ? (vb - va) * (plot_x1 - plot_x0) / (drawn_to_ - drawn_from_) : 0;
    const bool visible = width >= curve_min, wide = width > curve_reach, whole = va == a && vb == b;
    const i32 cx = (view_x(va) + view_x(vb)) / 2, last = static_cast<i32>(kMotionSamples) - 1;
    // The motion value at k of `of` along the visible window (read between the two samples about it).
    auto value_of = [&](const u32* v, i32 k, i32 of) -> u32 {
        const i32 u = va + (vb - va) * k / of;
        if (whole && k * last % of == 0) return v[k * last / of];
        const i32 pos = (u - a) * (last * 16) / (b - a);  // Q4 of a sample
        const u32 i = static_cast<u32>(pos >> 4), f = static_cast<u32>(pos & 15);
        return i >= static_cast<u32>(last) ? v[last] : (v[i] * (16 - f) + v[i + 1] * f + 8) / 16;
    };
    // Sample k of `of` along the visible window, relative to a node at x `centre`.
    auto point = [&](const u32* v, i32 k, i32 of, i32 centre) {
        draw::Point p;
        p.x = (view_x(va + (vb - va) * k / of) - centre) / 2;
        p.y = (plot_up(value_of(v, k, of), tl_dy) - tl_cy * unit) / 2;
        return p;
    };
    auto drawn = [&](u32 lane) {
        const DraftLane& l = draft_.lanes[lane];
        return visible && (lane == lane_ ? l.gesture != kNoGesture && l.selection : lane_moves(l));
    };
    u32 v[kMotionSamples];
    const u32 half = points / 2;
    // The other lanes: paths hidden always, their sketches below.
    flat_ribbon(frame.paths[n_other]), flat_ribbon(frame.paths[n_other_b]);
    draw::hide(frame, n_other), draw::hide(frame, n_other_b);
    {
        u32 slot = 0;
        for (u32 lane = 0; lane < kLanes; ++lane) {
            if (lane == lane_) continue;
            const u32 first = n_sketch + slot++ * sketch_pieces;
            if (!drawn(lane)) {
                for (u32 k = 0; k < sketch_pieces; ++k) draw::hide(frame, first + k);
                continue;
            }
            const DraftLane& l = draft_.lanes[lane];
            motion(l, draft_.length, v);
            const u32 shade = l.muted ? s_dim : s_other;
            const i32 of = static_cast<i32>(sketch_pieces);
            auto at = [&](i32 k) {
                draw::Point p;
                p.x = view_x(va + (vb - va) * k / of);
                p.y = plot_up(value_of(v, k, of), tl_dy);
                return p;
            };
            // Each piece from sample k's place to sample k + 1's (the motion's own 32 samples: with 31 pieces, sample k exactly).
            draw::Point from = at(0);
            for (i32 k = 0; k < of; ++k) {
                const draw::Point to = at(k + 1);
                draw::segment(frame, first + static_cast<u32>(k), from, to, sketch_length, shade, draw::fill_and_stroke);
                from = to;
            }
        }
        for (; slot < sketch_lanes; ++slot)
            for (u32 k = 0; k < sketch_pieces; ++k) draw::hide(frame, n_sketch + slot * sketch_pieces + k);
    }
    // The lane shown: its paths, as before.
    {
        const u32 first = n_shape, second = n_shape_b;
        const DraftLane& l = lane();
        if (!drawn(lane_)) {
            flat_ribbon(frame.paths[first]), flat_ribbon(frame.paths[second]);
            draw::hide(frame, first), draw::hide(frame, second);
            return;
        }
        motion(l, draft_.length, v);
        const u32 shade = l.muted ? s_dim : s_bright;
        if (wide) {
            for (u32 h = 0; h < 2; ++h) {  // each half about its own middle, as the shape page's
                const u32 node = h == 0 ? first : second;
                const i32 k0 = static_cast<i32>(h * (half - 1)), of = static_cast<i32>(2 * (half - 1));
                const i32 middle = (view_x(va + (vb - va) * k0 / of) + view_x(va + (vb - va) * (k0 + static_cast<i32>(half) - 1) / of)) / 2;
                draw::Point top[half];
                for (u32 i = 0; i < half; ++i) top[i] = point(v, k0 + static_cast<i32>(i), of, middle);
                ribbon(frame.paths[node], top, 7 + 2 * lane_ + h);
                draw::place(frame, node, middle, tl_cy * unit, 2 * unit, shade, draw::fill_and_stroke);
            }
            return;
        }
        draw::Point top[half];
        for (u32 i = 0; i < half; ++i) top[i] = point(v, static_cast<i32>(i), static_cast<i32>(half - 1), cx);
        ribbon(frame.paths[first], top, lane_);
        draw::place(frame, first, cx, tl_cy * unit, 2 * unit, shade, draw::fill_and_stroke);
        flat_ribbon(frame.paths[second]);
        draw::hide(frame, second);
    }
}

// The lane letters, a then b then c (build 060), baseline-left at (x, y) Q10 pixels, drawn at `scale` (Q10; build 052: 3/2, at the M1
// anchor, so they read against the 20 px marks; every tenth-of-a-pixel offset below scales by it, and the nodes'
// scale is it): the lane shown white, the others dim (s_mid). Each a ring, then a stem drawn over its fill; c one
// filled glyph, a ring open on the right (page_svg.py n_letter_c), in the fill shades, not the ring ones.
// Then encoder 1's dot (its tap switches the lane), enc1_dx right, level with the bowls.
template <class F> void VariationPage::draw_letters(F& frame, i32 x, i32 y, i32 scale) {
    using namespace screen;
    namespace draw = opxy::draw;
    const i32 X = x, Y = y, bowl_y = Y - x_height10 * scale / 20;
    const bool a_shown = lane_ == 0, b_shown = lane_ == 1, c_shown = lane_ == 2;
    // a: its bowl, the stem on the bowl's right to the x-height.
    const i32 ax = X + (ring_r10 + 5) * scale / 10;
    draw::place(frame, n_ring_a, ax, bowl_y, scale, a_shown ? s_ring_white : s_ring_mid, draw::fill_and_stroke);
    draw::place(frame, n_stem_a, ax + ring_r10 * scale / 10, Y - stem_a10 * scale / 20, scale, a_shown ? s_white : s_mid,
                draw::fill_and_stroke);
    // b: the stem on the left to the ascender, its bowl to the right.
    const i32 bx = X + letter_pitch * scale;
    draw::place(frame, n_stem_b, bx + stem_w10 * scale / 20, Y - stem_b10 * scale / 20, scale, b_shown ? s_white : s_mid,
                draw::fill_and_stroke);
    draw::place(frame, n_ring_b, bx + (ring_r10 + 5) * scale / 10, bowl_y, scale, b_shown ? s_ring_white : s_ring_mid,
                draw::fill_and_stroke);
    // c (build 060): a bare bowl, no stem. Build 062: placed by gap (build 064: c_centre100, 1.8 px tighter than a-b), so the gap after b's bowl matches
    // the gap after a's stem; a uniform pitch made it 0.8 px wider. Build 064 ("c is still just a little too far over to the right"): c_centre100,
    // 1.8 px tighter to b than a is to b, the row recentred on its ink.
    draw::place(frame, n_letter_c, X + c_centre100 * scale / 100, bowl_y, scale, c_shown ? s_white : s_mid,
                draw::fill_and_stroke);
    // Build 054 ("why is there a small hollow circle next to a/b"): encoder 1's dot is gone with its tap - the
    // lane is M1's, marked by these letters, and a tap that only repeats a key is one more thing to explain.
    draw::hide(frame, n_enc1);
}

// The header row, both pages (build 022): the title (in `line`) centred, the
// join's scenes top left, as long a form as ends title_gap before the title.
// Returns where the title ends, Q10.
template <class F> i32 VariationPage::header_row(F& frame, char* line) {
    using namespace screen;
    namespace draw = opxy::draw;
    const i32 w = words::width(line, header_size), x = words::centred_x(240 * unit, line, header_size);
    draw::text(frame, t_title, x, header_y * unit, unit, line);
    i32 room = x - title_gap * unit - join_x * unit;
    if (room > join_width * unit) room = join_width * unit;
    join(line, room);
    draw::text(frame, t_join, join_x * unit, header_y * unit, unit, line);
    return x + w;
}

// The playhead: white inside the window, where it says this variation is the one heard now, else the text's grey.
// A hairline the plot's height at the song's place, on both pages (build 054: "on the shape screen can we not
// just use the same playhead we're using on the variation page, given the context is just that gesture"). The
// variation page draws it on the shape page's cursor node (authored 1 x 100, the plot's 92 px at scale 0.92); the
// shape page on n_playhead, authored the same, since its cursor node is the edit cursor there. On the shape page the
// two lines are told apart by shade: the playhead white, the cursor the text's grey.
template <class F> void VariationPage::draw_playhead(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    const i32 x = playhead_x();
    shown_x_ = x;
    const u32 shade = points_ || playhead_inside() ? s_white : s_text;
    if (points_) {
        if (x < 0) draw::hide(frame, n_playhead);
        else draw::place(frame, n_playhead, x * unit, plot_cy * unit, (plot_y1 - plot_y0) * unit / (2 * cursor_half), shade, draw::fill_and_stroke);
        return;
    }
    draw::hide(frame, n_playhead);
    if (x < 0) draw::hide(frame, n_cursor);
    else draw::place(frame, n_cursor, x * unit, tl_cy * unit, (plot_y1 - plot_y0) * unit / (2 * cursor_half), shade, draw::fill_and_stroke);
}

// Build 049 (2026-11-23): the variation page's timeline across the shape page's plot (see aim_view for the
// view): the grid, the variation's join and the other section boundaries, the as set line, the two curves and
// encoder 2's dot. The playhead is draw_playhead's, the scene names and bar numbers draw_texts'.
// No band behind the window: the palette has nothing between black and s_dim (the beat lines' grey, which the
// grid would lose in), so the window is its curve alone.
template <class F> void VariationPage::draw_timeline(const PageTick& tick, F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    aim_view();
    if (ease_view(tick.elapsed, tick.reset)) ++revision_;  // still moving: the next frame redraws
    const i32 span = drawn_to_ - drawn_from_;
    // Build 054: no ruler line in the artwork any more, so nothing to cover (n_start is unused, hidden on both
    // pages) and no bar ticks on either.
    draw::hide(frame, n_start);
    for (u32 i = 0; i < bar_count; ++i) draw::hide(frame, n_bars + i);
    draw::hide(frame, n_floor);
    for (u32 i = 0; i < handle_count; ++i) draw::hide(frame, n_handles + i);
    // The grid, as the shape page's: full height, bar lines s_grid, beat lines s_dim; a beat apart while that is at
    // least grid_min px, else a bar, 4 bars, 16 bars; never more than grid_count.
    i32 every = 4;
    while (every < 256 && every * (plot_x1 - plot_x0) * view_q < grid_min * span) every *= 4;
    while (every < 256 && (span + every * view_q - 1) / (every * view_q) > static_cast<i32>(grid_count)) every *= 4;
    {
        // Build 049: the window's extent, a faint band (n_band strips) behind the grid and the curve.
        // Build 056 ("it overlaps the scene marker"): its ends are inset by band_inset, the marker line's half width,
        // so a scene marker or the song's end line on its edge stays whole.
        const i32 a = view_x(draft_.offset * view_q) + band_inset * unit, b = view_x((draft_.offset + static_cast<i32>(draft_.length)) * view_q) - band_inset * unit;
        const i32 w = band_width * unit;
        u32 k = 0;
        if (b - a >= w) {
            for (i32 c = a + w / 2; k < band_count; c += w) {
                const bool last = c + w / 2 >= b;
                draw::place(frame, n_band + k++, last ? b - w / 2 : c, tl_cy * unit, unit, s_band, draw::fill_and_stroke);
                if (last) break;
            }
        }
        for (; k < band_count; ++k) draw::hide(frame, n_band + k);
    }
    {
        const i32 unit_q = every * view_q;
        i32 st = (drawn_from_ >= 0 ? (drawn_from_ + unit_q - 1) / unit_q : -(-drawn_from_ / unit_q)) * every;
        u32 g = 0;
        // Build 072 ("the last bar is only appearing as half the length it should be"): `<=`, so the line
        // resting exactly on the view's right edge is drawn. Strict, the final cell was left without its own
        // line and read as a part cell. The mirror of build 066, which made the same fix for a boundary on the
        // left edge; `draw_scene_names` and the song-end marker have been inclusive at their edges since 067.
        for (; st * view_q <= drawn_to_ && g < grid_count; st += every, ++g)
            draw::place(frame, n_grid + g, view_x(st * view_q), tl_cy * unit, unit, st % 16 ? s_beat : s_bar, draw::fill_and_stroke);
        for (; g < grid_count; ++g) draw::hide(frame, n_grid + g);
    }
    // The variation's own join, 2 px wide. Build 052: authored the plot's half height plus the 12 px pole above and 12 px
    // below (down to the ruler), centred where the plot is, in s_text (s_join is the strip's cell now).
    if (drawn_from_ <= 0 && drawn_to_ >= 0) draw::place(frame, n_join, view_x(0), tl_cy * unit, unit, s_text, draw::fill_and_stroke);
    else draw::hide(frame, n_join);
    // The other section boundaries in view, mid grey: the start of the section before, the later joins.
    u32 m = 0;
    auto boundary = [&](const Section& sec) {
        const i32 q = sec.start * view_q;
        // Build 066: `>=`, so a boundary resting exactly on the view's left edge is still drawn. With the start
        // clamp above, the song's start lands there, and `draw_scene_names` already labels it with `>=`:
        // strict here left the word "start" with no line beneath it.
        if (sec.start && q >= drawn_from_ && q < drawn_to_ && m < mark_count)
            draw::place(frame, n_marks + m++, view_x(q), tl_cy * unit, unit, s_marker, draw::fill_and_stroke);
        return q < drawn_to_;
    };
    each_section(boundary, view_to_ + 1);
    // Build 051: a scene line at the song's end too (often the plot's right edge: the view stops at the song).
    const i32 end_q = song_end() * view_q;
    if (song_end() > 0 && end_q > drawn_from_ && end_q <= drawn_to_ && m < mark_count)
        draw::place(frame, n_marks + m++, view_x(end_q), tl_cy * unit, unit, s_marker, draw::fill_and_stroke);
    for (; m < mark_count; ++m) draw::hide(frame, n_marks + m);
    place_as_set(frame);
    draw_curves(frame);
    // Build 054: encoder 2's dot is gone with its tap; "shape" over M3 says it in words.
    draw::hide(frame, n_enc2);
}

// Build 049 (2026-11-23): each section in view named along the plot's top, "scene N", left aligned
// name_inset right of where it starts (or the plot's left edge, its start out of view), in the slots t_label_a and
// t_label_b in order (build 052: t_target is gone; build 049 had three, the third empty). Left out: a section showing under name_room px, a name that
// would run past the plot, or one that would overlap the next name kept. On the song's start the forms stay "scene
// N, end" (the section before: the song's last) and "start, scene N" (the section after).
template <class F> void VariationPage::draw_scene_names(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    struct Name { i32 x, w; char text[draw::text_capacity]; bool in_window; };
    const u32 limit = 16;
    Name found[limit];
    u32 count = 0, k = 0;
    const i32 span = drawn_to_ - drawn_from_;
    auto visit = [&](const Section& sec) {
        const u32 index = k++;
        const i32 start = sec.start * view_q;
        if (start >= drawn_to_) return false;
        const i32 a = start > drawn_from_ ? start : drawn_from_, b = sec.end * view_q < drawn_to_ ? sec.end * view_q : drawn_to_;
        if (b <= a || (b - a) * (plot_x1 - plot_x0) / span < name_room || sec.scene >= kSceneCount || count >= limit) return true;
        Name& n = found[count];
        u32 len = 0;
        n.text[0] = 0;
        if (index == 1 && song_start()) words::put(n.text, len, "start, ");
        words::put(n.text, len, "scene ");
        words::number(n.text, len, sec.scene + 1);
        if (index == 0 && song_start()) words::put(n.text, len, ", end");
        n.x = view_x(a) + name_inset * unit;
        n.w = words::width(n.text, label_size);
        n.in_window = sec.start < draft_.offset + static_cast<i32>(draft_.length) && sec.end > draft_.offset;
        if (n.x + n.w <= plot_x1 * unit) ++count;
        return true;
    };
    each_section(visit, drawn_to_ / view_q + 1);
    // Build 051: "end" at the song's end line, ending name_inset left of it (nothing follows it).
    const i32 end_q = song_end() * view_q;
    Name end_name = {0, 0, "end", false};
    bool has_end = false;
    if (song_end() > 0 && end_q >= drawn_from_ && end_q <= drawn_to_) {  // build 067: inclusive both ends now the label can flip
        end_name.w = words::width(end_name.text, label_size);
        end_name.x = view_x(end_q) - name_inset * unit - end_name.w;
        has_end = end_name.x >= plot_x0 * unit;
        // Build 067 ("end disappears as I expect it's rendered off screen, when it's at the edge of the screen,
        // it needs to be on the inside of the bar"). "end" goes left of its line, so a line near the plot's left
        // edge put the word off the plot and it was dropped without a word. It takes the other side of its line
        // instead, which is the inside of the view either way.
        if (!has_end) {
            end_name.x = view_x(end_q) + name_inset * unit;
            has_end = end_name.x + end_name.w <= plot_x1 * unit;
        }
    }
    // Build 061: "start" at the song's start line, beginning name_inset right of it (nothing precedes it), the
    // mirror of "end". Not at the join itself (entry 0, start_q 0): the join's own line marks it and the first
    // section already reads "start, scene N".
    const i32 start_q = song_start_at() * view_q;
    Name start_name = {0, 0, "start", false};
    bool has_start = false;
    if (song_start_at() < 0 && start_q >= drawn_from_ && start_q <= drawn_to_) {  // build 067: as above
        start_name.w = words::width(start_name.text, label_size);
        start_name.x = view_x(start_q) + name_inset * unit;
        has_start = start_name.x + start_name.w <= plot_x1 * unit;
        // Build 067: the same the other way round. "start" goes right of its line, so a line near the plot's right
        // edge would have dropped it; it takes the left of its line instead.
        if (!has_start) {
            start_name.x = view_x(start_q) - name_inset * unit - start_name.w;
            has_start = start_name.x >= plot_x0 * unit;
        }
    }
    // From the right: a name stays unless it would run into the next one kept (or "end"); and build 061: not if it
    // would run back into "start".
    bool kept[limit] = {};
    i32 next_x = has_end ? end_name.x : plot_x1 * unit + name_inset * unit;
    const i32 floor_x = has_start ? start_name.x + start_name.w + name_inset * unit : plot_x0 * unit;
    for (u32 i = count; i-- > 0;) {
        kept[i] = found[i].x + found[i].w + name_inset * unit <= next_x && found[i].x >= floor_x;
        if (kept[i]) next_x = found[i].x;
    }
    // Build 049: two names at most, t_label_a and t_label_b (both 10 px); build 052: the 12 px third slot is gone.
    const u32 slots[2] = {t_label_a, t_label_b};
    u32 used = 0;
    // Build 061: "start" takes the left slot. With both ends in view the whole song is, and the scene names in
    // between are the first thing to give up: the two lines say more than one scene's name would.
    if (has_start) draw::text(frame, slots[used++], start_name.x, (tl_y0 + name_y) * unit, unit, start_name.text);
    // Build 061: one scene name beside whichever single line is shown - "end" as before, or now "start". With both
    // in view there is no room for one, and no need: the two lines place the song better than a name would.
    if (has_end != has_start) {
        // One name beside it: the first kept that the window covers, else the first kept.
        u32 pick = count;
        for (u32 i = 0; i < count && pick == count; ++i)
            if (kept[i] && found[i].in_window) pick = i;
        for (u32 i = 0; i < count && pick == count; ++i)
            if (kept[i]) pick = i;
        if (pick < count) draw::text(frame, slots[used++], found[pick].x, (tl_y0 + name_y) * unit, unit, found[pick].text);
    }
    for (u32 i = 0; i < count && used < 2 && !has_end && !has_start; ++i)
        if (kept[i]) draw::text(frame, slots[used++], found[i].x, (tl_y0 + name_y) * unit, unit, found[i].text);
    if (has_end && used < 2) draw::text(frame, slots[used++], end_name.x, (tl_y0 + name_y) * unit, unit, end_name.text);
    for (; used < 2; ++used) draw::text(frame, slots[used], left * unit, (tl_y0 + name_y) * unit, unit, "");
}

// Build 049 (2026-11-23): the bar lines in view numbered underneath, by bars from the join as the mockup does:
// the bar before the join is -1, -2 ..., the first bar after it 1 (the join's own line is the brightest, unnumbered),
// so the line a bar after the join carries 2: bar n's number sits at its start. A line each bar, or every 2nd, 4th
// ... so that lines are at least number_gap px apart (counted from the join).
//
// All in one text, t_number_a's artwork (build 052's name; 049's t_gesture), each
// number placed by padding with spaces measured with words::width so its left edge falls at its line + number_inset,
// within number_slack of it, never before the line (half a pixel clear of it at least). A text holds text_chars characters though (a space is a quarter of
// the text's height wide): where lines are far apart only as many numbers as that holds are placed. Those nearest the
// join: the run starts at the first line before it or after it, whichever places more (ties: after), unless every
// line fits from the first in view.
template <class F> void VariationPage::draw_bar_numbers(F& frame) {
    // Build 049 (2026-11-23, option a): the bar the window starts in under its start (left-aligned, number_inset
    // in) and the bar its last step is in. Bars count from the join: 1 the first after it, -1 the last before it.
    // Build 054: the end bar's own slot became a mark, and only the start bar showed. Build 056: both in the one
    // slot, t_number_a, as a range (below).
    // (The first cut spread every bar's number along one text with spaces; a slot's 23 characters left
    // one number on a 2-bar window and the spaces' widths let them drift from their lines.)
    using namespace screen;
    namespace draw = opxy::draw;
    auto bar_of = [](i32 step) { return step >= 0 ? step / 16 + 1 : -((-step - 1) / 16 + 1); };
    auto label = [](i32 bar, char* out) {
        u32 n = 0;
        out[0] = 0;
        if (bar < 0) words::put(out, n, "-");
        words::number(out, n, static_cast<u32>(bar < 0 ? -bar : bar));
    };
    const i32 y = (tl_y1 + number_y) * unit, first = draft_.offset, last = draft_.offset + static_cast<i32>(draft_.length) - 1;
    const i32 a = view_x(first * view_q), b = view_x((last + 1) * view_q);
    // Build 056 ("it's really difficult to understand why you can't resize a variation"): the window's bar range
    // in the one slot, "5-8" (the start bar alone when the first and last steps share a bar), ">" for the dash when
    // either end is negative ("-2>1": "-2-1" can't be read). A message shows in its place, centred, while it lasts.
    (void)b;
    char start[16];
    {
        u32 n = 0;
        const i32 from = bar_of(first), to = bar_of(last);
        label(from, start);
        n = words::length(start);
        if (to != from) {
            words::put(start, n, from < 0 || to < 0 ? ">" : "-");
            label(to, start + n);
        }
    }
    i32 sx = a + number_inset * unit;
    const i32 sw = words::width(start, label_size);
    if (sx + sw > plot_x1 * unit) sx = plot_x1 * unit - sw;
    if (message_) return;
    draw::text(frame, t_number_a, sx, y, unit, start);
}

template <class F> void VariationPage::draw_texts(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    char line[draw::text_capacity];
    u32 n = 0;
    auto clear_line = [&]() { n = 0, line[0] = 0; };
    const Draft& d = draft_;
    const DraftLane& l = lane();
    // Header: the title centred, the variation's number in the song; the join's scenes top left.
    title(line);
    n = words::length(line);
    header_row(frame, line);
    // Build 056: no track list in the header (its slot is the M2 mark now); the track keys' LEDs carry the
    // lane's tracks, and the strip's target column says "tracks / none" when it has none, " *" when the target misses some.
    // Which lane shows: the letters at the M1 mark (draw_letters; build 052: were under the join's scenes).
    // Build 054: with Shift held, "mute" takes M1's place and the letters stand down.
    if (shift_) {
        draw::hide(frame, n_ring_a), draw::hide(frame, n_stem_a), draw::hide(frame, n_ring_b), draw::hide(frame, n_stem_b);
        draw::hide(frame, n_letter_c);  // build 060
        draw::hide(frame, n_enc1);
    } else {
        draw_letters(frame, letters_x, letters_y * unit, letters_scale);
    }
    // Build 029: no large gesture or target over the shape: the strip
    // already names both (2026-11-22), and the shape takes their room.
    // Build 049: the timeline's scene names along the plot's top (the slots t_label_a and t_label_b) and its bar numbers
    // underneath (t_number_a).
    draw_scene_names(frame);
    draw_bar_numbers(frame);
    place_strip(frame);
    // The control strip (build 052; was the bottom row), a name and a value a column: move, gesture, target,
    // amount; with Shift held, the Shift layer: snap, length, rate, then blank, and "delete" over M4.
    const char* const snap_name = snap_ == 0 ? "step" : snap_ == 1 ? "beat" : "bar";
    char name[draw::text_capacity], value[draw::text_capacity];
    if (shift_) {
        strip_column(frame, 0, "snap", snap_name);
        length_parts(name, value);
        strip_column(frame, 1, name, value);
        strip_column(frame, 2, "rate", rate_text(lane().rate));
        strip_column(frame, 3, "", "");
        // Build 030: Shift + M4's delete named over M4, as stock's song page
        // names it (solid while Shift is held); 019-029 named it nowhere.
        // Build 048: back as in 046 (047 put mute here; mute is encoder 4's plain tap now).
        // Build 052: at M4's mark, 20 px (the strip's column 4 was "delete" / "press m4"), Shift only: plain M4
        // closes the page and never reaches it. Build 054: "mute" over M1 beside it, the lane's (its letters give way
        // while Shift is held).
        // Build 055: the marks are centred on their keys (words::centred_x), and dimmer (the slots are TEXT grey).
        draw::text(frame, t_mark, words::centred_x(m4_x, "delete", mark_size), m_y * unit, unit, "delete");
        // Build 056: no other mark under Shift ("mute" is plain M2 now, not Shift + M1).
        draw::text(frame, t_mark_side, m3_x, m_y * unit, unit, "");
        draw::text(frame, t_mark_m2, m2_x, m_y * unit, unit, "");
        return;
    }
    // Build 054: "shape" over M3, and M4 says what it does - it is the gesture that opened the page, so stock
    // closes the page on it and the extension never sees the press.
    // Build 055: "edit" over M3 (shape was the wrong name), centred on the key.
    draw::text(frame, t_mark, words::centred_x(m4_x, "back", mark_size), m_y * unit, unit, "back");
    draw::text(frame, t_mark_side, words::centred_x(m3_x, "edit", mark_size), m_y * unit, unit, "edit");
    // Build 056 ("where has mute gone? it should be the second item on both"): "mute" over M2, on both pages.
    draw::text(frame, t_mark_m2, words::centred_x(m2_x, "mute", mark_size), m_y * unit, unit, "mute");
    // Build 053: the name is "move", not "move bar" - the snap it moves by is the Shift layer's own column,
    // where it is read and changed; out here it only crowded the cell.
    clear_line();
    words::put(line, n, "move");
    // Values: where the window starts from the join, the gesture, target, amount.
    char move[draw::text_capacity];
    u32 mn = 0;
    move[0] = 0;
    // Build 053 (on 052: "on join" and "+n bars" don't fit): "join", and the unit a single letter after the
    // number, "+2b" / "-48s" (the snap it moves by is the Shift layer's). The window moves up to
    // kMaxOffset steps, 127 bars, so three digits have to fit: "-2047s" is 44 px of the cell's 55.
    if (!d.offset) {
        words::put(move, mn, "join");
    } else {
        const u32 s = static_cast<u32>(d.offset < 0 ? -d.offset : d.offset);
        words::put(move, mn, d.offset < 0 ? "-" : "+");
        words::number(move, mn, s % 16 ? s : s / 16);
        words::put(move, mn, s % 16 ? "s" : "b");
    }
    strip_column(frame, 0, line, move);
    strip_column(frame, 1, "gesture", lane_name(l));
    {
        // Build 032: a one-track lane's target is that track's ("track 3").
        const i32 only = single_track();
        clear_line();
        if (!l.selection) {
            // Build 056: the header's "no tracks" is gone; the strip says it where the target would be.
            words::put(line, n, "tracks");
        } else if (only < 0) {
            words::put(line, n, "target");
        } else {
            words::put(line, n, "track ");
            words::number(line, n, static_cast<u32>(only) + 1);
        }
        char target[draw::text_capacity];
        u32 tn = 0;
        target[0] = 0;
        if (!l.selection) words::put(target, tn, "none");
        else target_value(target, tn);
        strip_column(frame, 2, line, target);
    }
    clear_line();
    if (l.muted) {
        words::put(line, n, "muted");  // build 047: in place of the amount's percentage
    } else {
        words::number(line, n, (l.amount * 100 + 127) / 255);
        words::put(line, n, "%");
    }
    strip_column(frame, 3, "amount", line);
}

// The shape page: the lane's line across the plot (the two outlines, a half
// of the window each), its points (the one at the cursor larger and white),
// the cursor, the
// as set line, the beats and bars on the ruler and the song's place while it
// plays in the window.
template <class F> void VariationPage::draw_points(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    const DraftLane& l = lane();
    // Build 049: the grid and ruler are the window's own (`length`, so beats and bars stay where they play);
    // the line's samples, points and cursor are in line steps (`line` of them, a step of length / line window steps).
    const u32 length = draft_.length, line = line_length(length), half = points / 2;
    const i32 span = (plot_x1 - plot_x0) * unit, height = plot_y1 - plot_y0;
    // A step (k of `of`: a step of the window as k of `length`) across; a Q8 value up.
    auto across = [&](u32 k, u32 of) { return plot_x0 * unit + span * static_cast<i32>(k) / static_cast<i32>(of); };
    // The line's samples span step 0 to the last step (L - 1 of L).
    // (32-bit only: the window's fraction in 4096ths, then the span by it.)
    auto sample_x = [&](u32 k, u32 of) {
        const i32 f = static_cast<i32>((line - 1) * k * 4096 / (of * line));
        return plot_x0 * unit + span / 4 * f / 1024;
    };
    auto up = [&](u32 q8) { return plot_y1 * unit - static_cast<i32>(q8 * static_cast<u32>(height) / 255) * unit / 256; };
    auto ticks = [&](u32 k, u32 of) { return (line - 1) * kStepTicks * k / of; };
    auto line_at = [&](u32 at) { return l.count ? value_at(l.points, l.count, at) : kAsSet << 8; };
    draw::hide(frame, n_join), draw::hide(frame, n_start);
    flat_ribbon(frame.paths[n_other_b]), flat_ribbon(frame.paths[n_shape_b]);  // build 050: the timeline's second halves
    draw::hide(frame, n_other_b), draw::hide(frame, n_shape_b);
    // Build 060: the shape page draws only the lane's own line (as before: no other lane behind it), so the other
    // lanes' sketch pieces (the variation page's) are hidden here.
    for (u32 i = 0; i < sketch_lanes * sketch_pieces; ++i) draw::hide(frame, n_sketch + i);
    for (u32 i = 0; i < mark_count; ++i) draw::hide(frame, n_marks + i);
    for (u32 i = n_handles; i < n_handles + handle_count; ++i) draw::hide(frame, i);  // build 049: the variation page's handles, none now
    // Build 023: the as set line dashed (n_as_set, authored across the plot),
    // the variation page's solid floor hidden; the grid behind everything, a
    // line a beat from the plot's top to its foot, the bars brighter (bars only
    // where beats would come closer than grid_min px).
    draw::hide(frame, n_floor);
    place_as_set(frame);
    {
        u32 grid_every = span / unit * 4 / static_cast<i32>(length) >= grid_min ? 4 : 16;
        while (length / grid_every > grid_count) grid_every *= 4;  // build 049: a 64-bar window: a line every 4 bars
        u32 g = 0;
        for (u32 st = 0; st < length && g < grid_count; st += grid_every, ++g)
            draw::place(frame, n_grid + g, across(st, length), plot_cy * unit,
                        unit, st % 16 ? s_dim : s_grid, draw::fill_and_stroke);
        for (; g < grid_count; ++g) draw::hide(frame, n_grid + g);
    }
    // The line: 63 samples, the middle one shared by both halves.
    // Build 053 (2026-11-24: "I can't enter the curve page... I see two steps light, but nothing else"): each half's
    // node sits at the middle of the samples it draws, not at the plot's own half. The samples span step 0 to the
    // line's last step (L - 1 of L), so a short line ends short of the plot's right edge: on an 8 step line the second
    // half's first sample fell 67 px from the plot's middle-right, past the 64 px a path reaches at scale 2, which made
    // the whole frame invalid (draw::valid) and the device drew nothing at all.
    for (u32 h = 0; h < 2; ++h) {
        const u32 of = 2 * (half - 1), first = h * (half - 1);
        const i32 cx = (sample_x(first, of) + sample_x(first + half - 1, of)) / 2;  // Q10
        draw::Point top[points / 2];
        for (u32 i = 0; i < half; ++i) {
            const u32 k = first + i;
            top[i].x = (sample_x(k, of) - cx) / 2;
            // A lane with no tracks: flat, as set, and hidden.
            top[i].y = (up(l.selection ? line_at(ticks(k, of)) : kAsSet << 8) - plot_cy * unit) / 2;
        }
        ribbon(frame.paths[h], top, 7 + h);
        if (!l.selection) draw::hide(frame, h);
        else draw::place(frame, h, cx, plot_cy * unit, 2 * unit, l.muted ? s_dim : s_bright, draw::fill_and_stroke);
    }
    // The line's points.
    u32 k = 0;
    if (l.selection)
        for (; k < l.count && k < point_count; ++k) {
            const Point& p = l.points[k];
            const bool here = p.step == cursor_;
            draw::place(frame, n_points + k, across(p.step, line), up(static_cast<u32>(p.value) << 8),
                        here ? 3 * unit / 2 : unit, l.muted ? s_dim : here ? s_white : s_bright, draw::fill_and_stroke);
        }
    for (; k < point_count; ++k) draw::hide(frame, n_points + k);
    // Build 057 ("mute is doing nothing on the edit page"): a muted lane's line, points and cursor draw dim as the variation
    // page's do; the cursor one grey up (s_grid) and the point under it larger, so both stay told apart.
    draw::place(frame, n_cursor, across(cursor_, line), plot_cy * unit, unit, l.muted ? s_grid : s_text, draw::fill_and_stroke);
    // Build 054 ("that way we can get rid of smaller bar below"): no ruler and no ticks on this page either.
    // The grid behind the plot already carries the beats and bars, and the bar labels name its ends.
    // (n_bars stay in the artwork, hidden on both pages, so the nodes keep their numbers.)
    for (u32 b = 0; b < bar_count; ++b) draw::hide(frame, n_bars + b);
    draw_playhead(frame);
}

template <class F> void VariationPage::draw_points_texts(F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    char line[draw::text_capacity];
    u32 n = 0;
    auto place = [&](u32 slot, i32 x, i32 y) { draw::text(frame, slot, x, y * unit, unit, line); };
    auto centred = [&](u32 slot, i32 y, i32 size) { place(slot, words::centred_x(240 * unit, line, size), y); };
    auto clear_line = [&]() { n = 0, line[0] = 0; };
    const DraftLane& l = lane();
    const u32 length = draft_.length;
    const i32 span = (plot_x1 - plot_x0) * unit;
    auto across = [&](u32 step) { return plot_x0 * unit + span * static_cast<i32>(step) / static_cast<i32>(length); };
    // Header (build 022), as the variation page's: the title centred; the join's
    // scenes at join_x; the lane letters at the M1 mark (build 052; were under them), the shown one white
    // (draw_letters). Build 023: no encoder 2 dot (its tap still goes back).
    // Build 024: the lane's tracks top right (build 056: gone, the track keys' LEDs say them).
    clear_line();
    words::put(line, n, "edit shape");  // build 055: "edit shape" at the top of the shape page
    header_row(frame, line);
    draw::hide(frame, n_enc2);
    draw_letters(frame, letters_x, letters_y * unit, letters_scale);
    // Build 056: the track list is gone from the header (its slot is the M2 mark); the LEDs carry the tracks.

    // Build 054: "variation" over M3 (it goes back there) and "back" over M4 (stock's close, the page's own gesture).
    // With Shift held neither key does anything here, so neither is marked.
    // Build 055 ("going back from shape should take you to the variations page"): "back" moves to M3, which does
    // return to the variation page. M4 still closes the page to the song page (plain M4 is the song_page_m4 claim's
    // own gesture), but it is not marked here: "back" must mean one step back. Both blank with Shift held, as before.
    draw::text(frame, t_mark, m4_x, m_y * unit, unit, "");
    draw::text(frame, t_mark_side, words::centred_x(m3_x, "back", mark_size), m_y * unit, unit, shift_ ? "" : "back");
    // Build 056: "mute" over M2 here too, blank under Shift as the other marks are.
    draw::text(frame, t_mark_m2, words::centred_x(m2_x, "mute", mark_size), m_y * unit, unit, shift_ ? "" : "mute");
    // The target (build 023): centred under the title, fitted (build 024). A message shows in its place.
    // Build 052: in t_number_a, 10 px (was 12 px in its own slot) at points_target_y 50.
    words::fit(line, target_text_[0] ? target_text_ : kNone, target_width * unit, target_size);
    n = words::length(line);
    if (message_) clear_line(), words::put(line, n, message_);
    centred(t_number_a, points_target_y, target_size);
    // The ruler's first and last bar (beats in windows under 2 bars).
    const u32 per = length >= 32 ? 16 : 4, count = length / per;
    const char* const unit_name = per == 16 ? "bar " : "beat ";
    clear_line();
    words::put(line, n, unit_name);
    words::number(line, n, 1);
    place(t_label_a, across(0), bar_label_y);
    clear_line();
    words::put(line, n, unit_name);
    words::number(line, n, count);
    // Build 063 (windows to 256 bars): "bar 128" and "bar 256" are wider than "bar 64" and, at the last bar's left edge,
    // ran off the screen's right (480 px); the label is held within it.
    i32 label_x = across((count - 1) * per);
    const i32 label_w = words::width(line, label_size);
    if (label_x + label_w > 480 * unit) label_x = 480 * unit - label_w;
    place(t_label_b, label_x, bar_label_y);
    // The control strip (build 052; was the bottom row): step, value, curve, point; with Shift: snap, then blank, and
    // the removal's pair (build 064: name "remove", value "press"; see below).
    const i32 k = edit::find(l, cursor_);
    place_strip(frame);
    if (shift_) {
        strip_column(frame, 0, "snap", points_snap_ == 0 ? "step" : points_snap_ == 1 ? "beat" : "bar");
        strip_column(frame, 1, "", "");
        strip_column(frame, 2, "", "");
        // Build 030: Shift + a step key removes that step's point.
        // Build 064 ("'step key' 'remove' ... it's both a little too long, and apparently does nothing"; then
        // "let's use [press]"): the column is encoder 4's, so Shift + its press removes the point at the cursor (Shift +
        // a step key still does). Name "remove", value "press": "remove" is 50.5 px at the value's 15 px of a 55 px cell
        // (build 053 ruled against that crowding) but 33.6 px at the name's 10 px; "press" is 35.7 px at 15 px.
        strip_column(frame, 3, "remove", "press");
    } else {
        clear_line();
        // Build 071 ("if you change from step to beat or bar, shouldn't the number drop to reflect the quantity of
        // that unit?" and "when there's a 100/100 or more, it is still going to be too wide for the box"): the cursor
        // counts in the snap's unit, the total in the name ("step of 256", "beat of 64", "bar of 16") and the position
        // alone in the value. On a bar snap it is the bar the cursor is in (bar 1 at steps 0-15), as the ruler's "bar 1".
        const u32 sn = kSnaps[points_snap_];
        // Build 071: the unit and the line's total, with the position alone in the 15 px value below.
        // "step of 256" measures 48 px by the tests' device_like but 55.5 px by words::width, the page's own
        // estimate, against a 55 px cell - so on the device it could be cut, and since build 059 a cut word
        // ends "..." , which would eat the number. Without "of" it is 36 px and 42 px, inside both. The plural
        // ("bars 16") was the other candidate and was dropped: the Shift layer already reads "bars" for the
        // window's length, and one word meaning two things on a page is what "flat" was kept out of.
        // Build 076 ("it's squashed, but I'm thinking let's just get rid of the total, we don't need
        // it"). The name is the unit alone now. Build 071 put the line's total here and 073 bracketed it to
        // say it was a count; on the device it still read as crowded, and the total was never the thing being
        // read - the position is. The value below keeps it.
        words::put(line, n, points_snap_ == 0 ? "step" : points_snap_ == 1 ? "beat" : "bar");
        char name_buf[draw::text_capacity];
        for (u32 i = 0; i <= n; ++i) name_buf[i] = line[i];
        clear_line();
        words::number(line, n, cursor_ / sn + 1);
        strip_column(frame, 0, name_buf, line);
        clear_line();
        {
            const i32 pc = edit::percent(edit::value(l, cursor_));
            if (!pc) words::put(line, n, "as set");
            else words::put(line, n, pc < 0 ? "-" : "+"), words::number(line, n, static_cast<u32>(pc < 0 ? -pc : pc)), words::put(line, n, "%");
        }
        strip_column(frame, 1, "value", line);
        // Build 053 (on 052: "fast 100" and "straight" don't fit): the word moves into the name, as the window
        // length's unit does, and the value is the number alone; a straight run in is "flat".
        clear_line();
        const char* curve_name = "curve";
        if (k <= 0) {
            words::put(line, n, kNone);  // no point here, or the first: nothing runs into it
        } else {
            const i32 c = l.points[k].curve;
            if (!c) {
                words::put(line, n, "flat");
            } else {
                curve_name = c > 0 ? "slow" : "fast";
                words::number(line, n, static_cast<u32>(((c < 0 ? -c : c) * 100 + 63) / 127));
            }
        }
        strip_column(frame, 2, curve_name, line);
        clear_line();
        const u32 total = l.count;
        if (!total) {
            words::put(line, n, "none");
        } else {
            if (k >= 0) words::number(line, n, static_cast<u32>(k) + 1);
            else words::put(line, n, kNone);
            words::put(line, n, "/");  // build 053: as the step's
            words::number(line, n, total);
        }
        strip_column(frame, 3, "point", line);
    }
}

template <class F> void VariationPage::draw(const PageTick& tick, F& frame) {
    using namespace screen;
    namespace draw = opxy::draw;
    if (message_) {
        message_left_ -= tick.elapsed > 0 ? tick.elapsed : 0;
        if (message_left_ <= 0) message_ = nullptr, ++revision_;
    }
    const i32 x = playhead_x();
    if (!tick.reset && drawn_ == revision_) {
        // Only the playhead moved (or nothing): one node, when it moved a pixel.
        if (x != shown_x_) draw_playhead(frame);
        return;
    }
    drawn_ = revision_;
    char line[draw::text_capacity];
    if (!around_.song) {
        // Nowhere to put a variation: say so, and nothing else.
        for (u32 p = 0; p < paths; ++p) flat_ribbon(frame.paths[p]);  // the artwork's flat ribbon, hidden
        for (u32 i = 0; i < nodes; ++i) draw::hide(frame, i);
        // (Build 052: the strip's cells and the chevrons are nodes too, so hidden here with the rest.)
        for (u32 i = 0; i < texts; ++i) draw::text(frame, i, left * unit, header_y * unit, unit, "");
        draw::copy(line, sizeof line, "no song");
        // Build 049: in the title's slot, 12 px (the 30 px headline had no other user since build 029). Build 052: 18 px
        // lower than 049's 68 and 186, as the timeline is, to stay centred between the strip and the M row.
        draw::text(frame, t_title, words::centred_x(240 * unit, line, header_size), no_song_title_y * unit, unit, line);
        draw::copy(line, sizeof line, "song is off");
        draw::text(frame, t_number_a, words::centred_x(240 * unit, line, message_size), message_y * unit, unit, line);
        return;
    }
    if (points_) {
        for (u32 i = 0; i < band_count; ++i) draw::hide(frame, n_band + i);  // build 049: the variation page's band
        draw_points(frame);
        draw_points_texts(frame);
        return;
    }
    for (u32 i = n_cursor; i < nodes; ++i) draw::hide(frame, i);  // the shape page's
    for (u32 i = 0; i < mark_count; ++i) draw::hide(frame, n_marks + i);
    draw_timeline(tick, frame);
    draw_playhead(frame);
    draw_texts(frame);
    if (message_) {  // build 054: in the bar numbers' slot (draw_bar_numbers leaves it), centred under the chart
        draw::copy(line, sizeof line, message_);
        draw::text(frame, t_number_a, words::centred_x(240 * unit, line, message_size), message_y * unit, unit, line);
    }
}
}
