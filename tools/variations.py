#!/usr/bin/env python3
"""Variations' saved bytes (format version 12; versions 1-11 still read) to and from JSON.

The bytes are the `variations` extension's entry in a project companion's
`open-xy.extensions` section. Put them on a project with:

    python tools/variations.py encode song.json song.bin
    python ../open-xy/sdk.py project state song.xy --extension variations --set song.bin

JSON form (every field required unless marked):

    {"variations": [{
        "song": 1, "entry": 3, "offset": -32, "length": 32,
        "lanes": [
            {"gesture": "break", "amount": 200, "target": "volume",
             "tracks": [1, 2], "default_tracks": true, "default_target": true,
             "line": [{"step": 0, "value": 128}, {"step": 31, "value": 0, "curve": 127}]},
            {"gesture": "rise", "amount": 128, "target": "cutoff", "tracks": [5]}
        ]
    }]}

Songs, entries, scenes and tracks are numbered as on the OP-XY (from 1); steps
within the window from 0. "curve" is optional (0, straight).

A variation has three lanes, A, B and C ("lanes" holds one to three; a missing lane
is empty; version 11, build 060: "what about a third lane then"). A lane moves one target on its own tracks: "target" is a name
from TARGETS ("volume", "cutoff", "lfo speed", "notes", "param 1" for the
engine's first control, slot by slot: "m1 1" is still read), a track word (an
integer) or null (none). "control" (optional, version 6, build 025; an engine
target only): the engine control's name ("tone"): the lane moves the control
of that name on each of its tracks whose engine has one, wherever it is, and
leaves the others alone; its "target" is then the slot it was chosen in. A
lane's point values are 0..255 with 128 "as set" (the knob where it is), 0
its counter-clockwise end, 255 its clockwise end. "gesture" is one of
GESTURES or "none" (an empty lane). "curve" (optional: the gesture's own,
DEFAULT_CURVES) bends the gesture's shape, -127..127 as a point's curve (above
0 a slow start, below 0 a fast start). "default_tracks" (optional, false): the
tracks follow the gesture and position; "default_target" (optional: true
when "target" is left out, which then takes the gesture's: volume for
build, break, drop, cut in and gate, notes for stutter (a note retrigger, build 046; its line the density, build 047), cutoff for the rest). "fade in" and "fade out" start
on the volume and take any target (build 039; builds 016-038 fixed them to it,
and this tool went on refusing any other until build 062).
"edited" (optional, false; lane flags bit 2): the shape page (build 019: the points page) has edited the
gesture's points, so "line" is the lane's own (the page shows it as custom,
and regenerates it from the gesture when its gesture, amount, curve or tracks
change).

Version 5 (build 024): a lane has ONE line, "line" (its points), and every
one of its tracks follows it; build's and break's tracks follow it one after
another (stagger(): build the lowest track first, break the highest, as far
apart as the amount spreads them), the line being the leading track's.

Version 7 (build 041): "rate" (optional, 1; 1, 2, 4 or 8) is lane flags bits 4-5, log2 of how many
times the lane's line plays across the window: the points are one cycle over the whole length, and at
rate N the window time t reads the line at (t * N) mod span. Version 6 bytes decode as version 7 (rate 1).

Version 8 (build 047): a lane's "muted" (optional, false; lane flags bit 6) is kept and edited but plays
nothing, and a variation's "reach" (optional, []; header byte 6 bit 2) lists the later joins its window
crosses, 1..15 of {"scene": n (1-based, or null), "start": steps}: start is the distance in steps from the
variation's own join to the join into the next entry, strictly rising, 1..1023. A length may now be 256
steps (was 128). Version 5-7 bytes decode as version 8 (no muted lanes, no reach).

Version 9 (build 049): a length may be up to 1024 steps (64 bars), a reach lists up to 31 joins (starts up
to 4095), and a line has its own length: a point's step is below line_length(length) = min(length, 256),
and over a longer window the line is stretched, window time t reading it at t * 256 / length. Version 8
bytes (within the old ranges) are valid version 9 bytes and decode as version 9.

Version 12 (build 063: "I'd like a song spanning variation to be an option"): a length may be up to 4096
steps (256 bars), a reach and a back list up to 95 joins each (starts up to 12286), an offset -8191..8191; the
bytes are laid out as before. Version 9-11 bytes (within the old ranges) are valid version 12 bytes and decode as
version 12.

Version 10 (build 058): a variation's "back" (optional, []; header byte 6 bit 3, after the reach list) lists the
earlier joins its window's start crosses before its own, 1..31 of {"scene": n (1-based, or null), "start": steps}:
scene is that of the entry j before the variation's own, start the distance in steps forward from the join into
that entry to the variation's own join, strictly rising with j, 1..4095; the window must start before the last
of them (offset + start <= 0; build 064: was < 0). Version 8 and 9 bytes decode as version 10 (no back list).

Version 6 (build 025) is version 5 with "control": a lane's flags bit 3 and
the name (u8 length 1..15, lowercase characters) after its curve. Version 5
bytes decode as version 6 (the same bytes, no names).

"scenes" is optional: [scene before the join, scene after it] (each a number
or null for not known; absent: neither known, as version 1 data). The
extension keeps a variation on the join with these scenes when the song is
edited, and fills them in when they are not known. "unplaced" (optional,
false): no join of the song has those scenes any more, so the variation is
kept but not played.

"line" is optional: without it a gesture other than custom gets the points
the variation page would give it (lane_line, the mirror of gestures.hpp).

Version 1-4 bytes decode as version 6, as the extension rewrites them
(upgrade(), the mirror of store.hpp's): from versions 1 and 2, first as
version 4 (lane A the line the gesture moved, lane B a second line with
points, a third and fourth a second variation on the same window); from
version 3, each lane its gesture's own curve; then each lane's one line, its
leading track's (or the lowest track's with points), build and break (unless
edited) made again from the gesture so the stagger reproduces version 4's
per-track entries and exits.
"""
import json
import struct
import sys

MAGIC = b"VR"
VERSION = 12
LANES = 3  # build 060 (version 11): lanes A, B and C
LANES_V10 = 2  # version 10 and below hold two; they load with lane C empty (store.hpp kLanesV10)
RATE_SHIFT = 4  # lane flags bits 4-5 (version 7): log2 of the lane's rate
RATE_MASK = 0x30
RATES = (1, 2, 4, 8)
MUTED = 0x40  # lane flags bit 6 (version 8): kept and edited, plays nothing
REACHING = 4  # variation header byte 6 bit 2 (version 8): a reach list follows the header
BACK_REACHING = 8  # variation header byte 6 bit 3 (version 10, build 058): a backward reach list follows the forward one
# Build 063 (version 12: "I'd like a song spanning variation to be an option"): a song of 2-bar scenes holds up to
# 96 entries, so 95 joins each way, in windows of up to 256 bars; the mirror of store.hpp's constants.
MAX_REACH = 95  # 31 until version 12, 15 until version 9
MAX_LENGTH = 4096  # steps (1024 until version 12, 256 in version 8, 128 before)
MAX_OFFSET = 2 * MAX_LENGTH - 1  # 8191 (2047 until version 12): steps from the join, into the section after; version 5 also as far back into the one before
MAX_REACH_START = MAX_OFFSET + MAX_LENGTH - 1  # 12286 (4095 until version 12): the furthest join a longest, latest window crosses
# What versions 9-11 allowed (a store of those versions is read within these, as before).
MAX_OFFSET_V11, MAX_LENGTH_V11, MAX_REACH_V11, MAX_REACH_START_V11 = 2047, 1024, 31, 4095
NAMED = 8  # lane flags bit 3 (version 6): an engine control by name
NAME_CHARS = 15
NO_SCENE = 0xff
SCENES = 99
CAPACITY = 8192
MAX_VARIATIONS = 64
TRACKS = 8
GESTURES = ["custom", "build", "break", "drop", "rise", "fall", "swell", "dip", "fade in", "fade out",
            "random", "wander", "cut in", "gate", "stutter", "stutter in", "spring", "stutter out", "hold"]  # build 062: hold ("flat line / silence curve")
# Build 016 fixed the fades to the volume; build 039 freed them, so this names only where they start.
FADES = ("fade in", "fade out")  # build 062: their default target, not a restriction
NO_GESTURE = 0xff
NOTES = 0x40
NO_TARGET = 0xff
# The SDK's track_targets (opxy/extension.hpp): word -> name, the engine's
# controls by position; test_variations_tool.py checks them against the header.
TARGETS = {"notes": NOTES}
TARGETS.update({f"param {w + 1}": w for w in range(8)})
TARGET_ALIASES = {f"m1 {w + 1}": w for w in range(8)}  # build 024's names for the slots
TARGETS.update({"attack": 8, "decay": 9, "sustain": 10, "release": 11, "volume": 15,
                "filter attack": 32, "filter decay": 33, "filter sustain": 34, "filter release": 35,
                "cutoff": 16, "resonance": 17, "env amount": 18, "key track": 19,
                "ext send": 20, "tape send": 21, "fx i send": 22, "fx ii send": 23,
                "lfo speed": 24, "lfo amount": 25, "pan": 40})
TARGET_NAMES = {w: n for n, w in TARGETS.items()}
ENGINE_WORDS = 8
VOLUME, CUTOFF, SEND = 15, 16, 22
# Version 1 and 2.
LEGACY_GESTURES = ["custom", "build", "break", "drop", "swell", "filter rise", "filter fall", "clear"]
GESTURES_FROM_LEGACY = [0, 1, 2, 3, 4, 4, 5, NO_GESTURE]
LINES = ["level", "notes", "send", "filter"]
LINE_TARGETS = [VOLUME, NOTES, SEND, CUTOFF]
HEADER, LANE_HEADER, LANE_HEADER_V3 = 9, 7, 6


class FormatError(ValueError):
    pass


def _need(ok, why):
    if not ok:
        raise FormatError(why)


def _int(value, low, high, what):
    _need(isinstance(value, int) and not isinstance(value, bool), f"{what} must be an integer")
    _need(low <= value <= high, f"{what} must be {low}..{high}, not {value}")
    return value


def natural_target(gesture):
    """The target a gesture moves while the lane's target follows it (gesture an index or NO_GESTURE)."""
    if gesture == 18:  # build 062 hold ("flat line"): the volume to start, any target after
        return VOLUME
    if 14 <= gesture <= 17:  # the stutters (builds 046, 048): shown as notes
        return NOTES
    if gesture in (8, 9, 12, 13):  # fade in, fade out, cut in, gate: the volume
        return VOLUME
    return NO_TARGET if gesture == NO_GESTURE else CUTOFF if gesture >= 4 else VOLUME


# The gesture generator: gestures.hpp's generate(), integer for integer.
SUDDEN, SLOW_START, FAST_START, EASE_OUT, EASE_IN, SWELL_CURVE = 127, 60, -60, -40, 40, 40
AS_SET = 128
# Each gesture's own curve (store.hpp default_curve): rise a slow start, fall a fast one, swell and dip
# out fast (and back slow), build and break evenly spread; drop and custom none.
DEFAULT_CURVES = {"rise": SLOW_START, "fall": FAST_START, "swell": EASE_OUT, "dip": EASE_OUT,
                  "fade in": SLOW_START, "fade out": FAST_START}


def default_curve(gesture):
    """A gesture's own curve, by name or index (NO_GESTURE: 0)."""
    if isinstance(gesture, int):
        gesture = GESTURES[gesture] if gesture < len(GESTURES) else "none"
    return DEFAULT_CURVES.get(gesture, 0)


# line.hpp's bend(), integer for integer: f ** 2^(2c), c = curve / 127, f Q16.
_LOG2 = [0, 2909, 5732, 8473, 11136, 13727, 16248, 18704, 21098, 23433, 25711, 27936, 30109, 32234, 34312,
         36346, 38336, 40286, 42196, 44068, 45904, 47705, 49472, 51207, 52911, 54584, 56229, 57845, 59434,
         60997, 62534, 64047, 65536]
_EXP2 = [65536, 66971, 68438, 69936, 71468, 73032, 74632, 76266, 77936, 79642, 81386, 83169, 84990, 86851,
         88752, 90696, 92682, 94711, 96785, 98905, 101070, 103283, 105545, 107856, 110218, 112631, 115098,
         117618, 120194, 122825, 125515, 128263, 131072]
ONE = 65536


def _neg_log2(f):
    whole = 0
    while f < ONE:
        f <<= 1
        whole += 1
    frac = f - ONE
    i, rest = frac >> 11, frac & 2047
    return (whole << 16) - (_LOG2[i] + ((_LOG2[i + 1] - _LOG2[i]) * rest >> 11))


def _exp2_neg(x):
    whole = x >> 16
    if whole >= 16:
        return 0
    frac = (ONE - (x & 0xffff)) & 0xffff
    shift = whole + (1 if frac else 0)
    i, rest = frac >> 11, frac & 2047
    return (_EXP2[i] + ((_EXP2[i + 1] - _EXP2[i]) * rest >> 11)) >> shift


def _exponent(curve):
    x = abs(2 * curve * 65536) // 127 * (1 if curve >= 0 else -1)  # C's division: towards zero
    if x >= 0:
        return _exp2_neg((2 << 16) - x) << 2
    return _exp2_neg(-x)


def bend(f, curve):
    """line.hpp's bend: f (Q16) ** 2^(2 curve / 127), Q16."""
    if curve == 0 or f == 0 or f >= ONE:
        return f
    e = _exponent(curve)
    l = _neg_log2(f)
    x = (((l * (e >> 10)) & 0xffffffff) >> 6) + (((l * (e & 1023)) & 0xffffffff) >> 16)
    return _exp2_neg(x & 0xffffffff)


def turn_step(r, n, amount, length, curve=0):
    """Where the r-th of n tracks enters (build) or leaves (break), in steps; the curve bends the spread."""
    if not curve:
        num = length * (n * 255 + (2 * r + 1) * amount) - length * n * amount
        den = 2 * n * 255
    else:
        p = bend(((2 * r + 1) << 16) // (2 * n), -curve)
        num, den = length * (32768 * (255 - amount) + p * amount), 65536 * 255
    step = (num + den // 2) // den
    grid = 4 if length >= 16 else 1
    step = (step + grid // 2) // grid * grid
    return min(max(step, grid), length - grid)


class Rng:
    """gestures.hpp's Rng: xorshift32 seeded with seed ^ 0x9e3779b9 (1 if that is 0)."""

    def __init__(self, seed):
        self.s = (seed ^ 0x9E3779B9) & 0xFFFFFFFF or 1

    def next(self):
        s = self.s
        s ^= (s << 13) & 0xFFFFFFFF
        s ^= s >> 17
        s ^= (s << 5) & 0xFFFFFFFF
        self.s = s
        return s

    def around(self, depth):
        """As set plus or minus up to depth (0..127), uniform."""
        # Build 042: a 16-bit fraction scaled by the depth: a new amount keeps the pattern.
        return AS_SET + ((self.next() >> 16) * (2 * depth + 1) >> 16) - depth


def line_length(length):
    """line.hpp: a line's own length (build 049): the window's up to 256 steps, 256 stretched over a longer one."""
    return min(length, 256)


def stutter_scale(length):
    """Build 044: window steps per step of stutter's stored bar (repeated every bar)."""
    return length // 16 if length > 16 else 1


def drop_gap(amount, length):
    """Build 049: real time (4..16 window steps); on a stretched line (window past 256) in line steps, at least 1."""
    gap = min(4 + (amount * 12 + 127) // 255, length - 1)
    return max(gap * 256 // length, 1) if length > 256 else gap


def up_by(a):
    return AS_SET + (a * 127 + 127) // 255


def down_by(a):
    return AS_SET - (a * 128 + 127) // 255


def count_bits(mask):
    return bin(mask).count("1")


def stagger_rank(gesture, selection, track):
    """stagger.hpp: a track's place in the stagger (0 the leading track)."""
    if gesture == 2:  # break: the highest first
        return count_bits(selection >> track >> 1)
    return count_bits(selection & ((1 << track) - 1))


def leading_track(gesture, selection):
    """stagger.hpp: the leading track (0-7), or -1 for none."""
    for t in range(TRACKS):
        track = 7 - t if gesture == 2 else t
        if selection >> track & 1:
            return track
    return -1


def stagger(gesture, amount, curve, selection, track, length):
    """stagger.hpp: how many line steps later than the lane's line `track` (0-7) follows it (gesture an index);
    spread over the line's length (build 049)."""
    length = line_length(length)
    if gesture not in (1, 2) or not selection >> track & 1:
        return 0
    n, r = count_bits(selection), stagger_rank(gesture, selection, track)
    return turn_step(r, n, amount, length, curve) - turn_step(0, n, amount, length, curve) if r else 0


def entry_line(gesture, amount, curve, selection, length):
    """stagger.hpp: build's or break's line (the leading track's jump), as point dicts (over the line's length)."""
    length = line_length(length)
    n = count_bits(selection)
    at = turn_step(0, n or 1, amount, length, curve)
    start, end = (0, AS_SET) if gesture == 1 else (AS_SET, 0)
    pts = [{"step": 0, "value": start}]
    if at > 1:
        pts.append({"step": at - 1, "value": start})
    pts.append({"step": at, "value": end, "curve": SUDDEN})
    return pts


def lane_line(gesture, amount, window_length, tracks, curve=None, seed=0):
    """A lane's one line as the page makes it (gestures.hpp generate): a list of points. Tracks from 1.
    Build 049: in line steps, over line_length(window_length); the drop's gap is real time (drop_gap).
    curve: the lane's (None: the gesture's own). seed: random and wander's dice (memory only on the page)."""
    if curve is None:
        curve = default_curve(gesture)

    def point(step, value, curve=0):
        p = {"step": step, "value": value}
        if curve:
            p["curve"] = curve
        return p

    length = line_length(window_length)
    selection = sum(1 << (t - 1) for t in set(tracks))
    if not selection:
        return []  # no tracks: no line (gestures.hpp)
    a = amount
    if gesture in ("build", "break"):
        return entry_line(GESTURES.index(gesture), a, curve, selection, window_length)
    if gesture == "drop":
        at = length - drop_gap(a, window_length)
        pts = [point(0, AS_SET)]
        if at > 1:
            pts.append(point(at - 1, AS_SET))
        return pts + [point(at, 0, SUDDEN)]
    if gesture in ("random", "wander"):
        if a == 0:
            return []
        rng = Rng(seed)
        depth = up_by(a) - AS_SET
        if gesture == "random":
            # Sample and hold: each value held to the step before the next, then a jump.
            every = 16 if length > 128 else 8 if length > 64 else 4
            held = rng.around(depth)
            pts = [point(0, held)]
            for s in range(every, length, every):
                pts.append(point(s - 1, held))
                held = rng.around(depth)
                pts.append(point(s, held, SUDDEN))
            return pts
        pts = [point(0, AS_SET)]
        pts += [point(s, rng.around(depth)) for s in range(16 if length > 128 else 8, length - 1, 16 if length > 128 else 8)]
        return pts + [point(length - 1, AS_SET)]
    if gesture == "cut in":
        gap = drop_gap(a, window_length)
        pts = [point(0, 0)]
        if gap > 1:
            pts.append(point(gap - 1, 0))
        return pts + [point(gap, AS_SET, SUDDEN)]
    if gesture == "gate":
        # Build 044: a gate over the whole window, a beat on and a beat off, starting on; the rate its
        # speed (playback), the amount the off beats' dip. One bar stored, stretched over the window's
        # steps (stutter_scale), repeated every bar in playback.
        if a == 0:
            return []
        cycle, scale = min(length, 16), stutter_scale(length)
        pts = []
        for k in range(0, (cycle + 3) // 4):  # each beat held to the step before the next, then a jump
            at = k * 4 * scale
            if k:
                pts.append(point(at - 1, AS_SET if k % 2 else down_by(a)))
            pts.append(point(at, down_by(a) if k % 2 else AS_SET, SUDDEN if k else 0))
        return pts
    if gesture == "stutter":  # build 048: steady, the top all the way
        return [point(0, 255), point(length - 1, 255)]
    if gesture == "stutter in":
        return [point(0, AS_SET), point(length - 1, 255, SLOW_START)]
    if gesture == "stutter out":
        return [point(0, 255), point(length - 1, AS_SET, FAST_START)]
    if gesture == "spring":
        # Build 047: its line is the density: a slow-start climb to the top at three quarters of the
        # window, then a damped spring back to none (steps not past the one before are left out).
        p = length * 3 // 4
        q = length - 1 - p
        pts = [point(0, AS_SET), point(p, 255, SLOW_START)]
        last = p
        for step, value in ((p + q // 4, 170), (p + q // 2, 225), (p + 3 * q // 4, 150), (length - 1, AS_SET)):
            if step > last:
                pts.append(point(step, value))
                last = step
        return pts
    if gesture in ("rise", "fall", "fade in", "fade out"):
        # Build 036: rise leaves as set, up by the amount (015-035 arrived from below, as fade in).
        arrives = gesture == "fade in"
        start, end = (down_by(a), AS_SET) if arrives else (AS_SET, up_by(a) if gesture == "rise" else down_by(a))
        return [point(0, start), point(length - 1, end, curve)] if start != end else []
    if gesture == "hold":
        # Build 062 ("flat line / silence curve"): a flat line at the amount's depth below as set, first step to last.
        depth = down_by(a)
        if depth == AS_SET:
            return []
        return [point(0, depth)] + ([point(length - 1, depth)] if length > 1 else [])
    if gesture in ("swell", "dip"):
        peak = up_by(a) if gesture == "swell" else down_by(a)
        return [point(0, AS_SET), point(length // 2, peak, curve), point(length - 1, AS_SET, -curve)] if peak != AS_SET else []
    return []


def track_values(lane, length, at):
    """What each of a decoded lane's tracks (from 1) asks at `at` steps into the window (Q8, as window.hpp's
    targets: each follows the line, build's and break's shifted by the stagger): {track: value}."""
    g = NO_GESTURE if lane["gesture"] == "none" else GESTURES.index(lane["gesture"])
    selection = sum(1 << (t - 1) for t in lane["tracks"])
    pts = lane["line"]
    out = {}
    if not pts:
        return out
    for t in lane["tracks"]:
        late = stagger(g, lane["amount"], lane["curve"], selection, t - 1, length)
        ticks = at * 480
        if length > 256:  # build 049: the line stretched over a long window
            ticks = ticks * 256 // length
        out[t] = value_at(pts, max(0, ticks - late * 480))
    return out


def value_at(points, at):
    """line.hpp's value_at on point dicts, Q8."""
    if at <= points[0]["step"] * 480:
        return points[0]["value"] << 8
    for i in range(1, len(points)):
        to = points[i]["step"] * 480
        if at > to:
            continue
        frm = points[i - 1]["step"] * 480
        f = ((at - frm) << 12) // ((to - frm) >> 4)
        k = bend(f, points[i].get("curve", 0))
        a, z = points[i - 1]["value"] << 8, points[i]["value"] << 8
        d = (z - a) * (k >> 1)
        return a + (d >> 15 if d >= 0 else -((-d + (1 << 15) - 1) >> 15))
    return points[-1]["value"] << 8


def _target(value, at):
    if value is None:
        return NO_TARGET
    if isinstance(value, str):
        if value in TARGET_ALIASES:
            return TARGET_ALIASES[value]
        _need(value in TARGETS, f"{at} target must be one of {sorted(TARGETS)}")
        return TARGETS[value]
    _need(isinstance(value, int) and value in TARGET_NAMES, f"{at} target {value!r} is no target")
    return value


def _name_ok(name):
    return 1 <= len(name) <= NAME_CHARS and all(0x20 <= ord(c) < 0x7f and not "A" <= c <= "Z" for c in name)


def _points(points, length, at):
    _need(1 <= len(points) <= 32, f"{at}: a line has 1..32 points")
    out, last = bytearray(), -1
    for p in points:
        step = _int(p["step"], 0, line_length(length) - 1, f"{at} point step")
        _need(step > last, f"{at}: points must be in step order, one a step")
        last = step
        out += struct.pack("<BBb", step, _int(p["value"], 0, 255, f"{at} point value"),
                           _int(p.get("curve", 0), -127, 127, f"{at} point curve"))
    return bytes(out)


def _lane(lane, length, at):
    gesture = lane.get("gesture", "none")
    _need(gesture == "none" or gesture in GESTURES, f"{at} gesture must be one of {GESTURES} or none")
    g = NO_GESTURE if gesture == "none" else GESTURES.index(gesture)
    amount = _int(lane.get("amount", 128), 0, 255, f"{at} amount")
    tracks = lane.get("tracks", [])
    _need(isinstance(tracks, list), f"{at} tracks must be a list")
    selection = 0
    for t in tracks:
        selection |= 1 << (_int(t, 1, TRACKS, f"{at} track") - 1)
    default_target = lane.get("default_target", "target" not in lane)
    target = natural_target(g) if "target" not in lane else _target(lane["target"], at)
    curve = _int(lane.get("curve", default_curve(gesture)), -127, 127, f"{at} curve")
    edited = bool(lane.get("edited", False)) and gesture != "none"
    _need("lines" not in lane, f"{at}: version 5 has one \"line\" a lane, not \"lines\"")
    if "line" in lane or gesture in ("custom", "none"):
        line = lane.get("line", [])
    else:
        line = lane_line(gesture, amount, length, tracks, curve, lane.get("seed", 0))
    rate = lane.get("rate", 1)
    _need(rate in RATES and not isinstance(rate, bool), f"{at} rate must be one of {list(RATES)}")
    control = lane.get("control")
    if control is not None:
        _need(isinstance(control, str) and _name_ok(control), f"{at} control must be 1..{NAME_CHARS} lowercase characters")
        _need(target < ENGINE_WORDS, f"{at}: a control by name needs an engine target (\"param 1\"..\"param 8\")")
    out = bytearray([g, amount, target, selection,
                     (1 if lane.get("default_tracks", False) else 0) | (2 if default_target else 0) | (4 if edited else 0) |
                     (NAMED if control else 0) | (RATES.index(rate) << RATE_SHIFT) | (MUTED if lane.get("muted", False) else 0)])
    out += struct.pack("<b", curve)
    if control:
        out += bytes([len(control)]) + control.encode("ascii")
    out += bytes([len(line)])
    if line:
        out += _points(line, length, at)
    return bytes(out)


def encode(doc):
    """JSON document -> version 12 bytes."""
    _need(isinstance(doc, dict) and isinstance(doc.get("variations"), list), "expected {\"variations\": [...]}")
    variations = doc["variations"]
    _need(len(variations) <= MAX_VARIATIONS, f"at most {MAX_VARIATIONS} variations")
    out = bytearray(MAGIC + bytes([VERSION, len(variations)]))
    for i, v in enumerate(variations):
        at = f"variation {i + 1}"
        length = _int(v["length"], 8, MAX_LENGTH, f"{at} length")
        offset = _int(v["offset"], -MAX_OFFSET, MAX_OFFSET, f"{at} offset")  # version 5: anywhere in the section before too
        scenes = v.get("scenes", [None, None])
        _need(isinstance(scenes, list) and len(scenes) == 2, f"{at} scenes must be [before, after]")
        scenes = [NO_SCENE if s is None else _int(s, 1, SCENES, f"{at} scene") - 1 for s in scenes]
        lanes = v["lanes"]
        _need(isinstance(lanes, list) and 1 <= len(lanes) <= LANES, f"{at} lanes must be a list of one to three")
        reach = v.get("reach", [])
        _need(isinstance(reach, list) and len(reach) <= MAX_REACH, f"{at} reach must be a list of at most {MAX_REACH}")
        stops = []
        for j, r in enumerate(reach):
            scene = r.get("scene")
            stop = (NO_SCENE if scene is None else _int(scene, 1, SCENES, f"{at} reach scene") - 1,
                    _int(r["start"], 1, MAX_REACH_START, f"{at} reach start"))
            _need(not stops or stop[1] > stops[-1][1], f"{at} reach starts must rise")
            stops.append(stop)
        _need(not stops or offset + length > stops[0][1], f"{at}: the window must cross its first reach join")
        back = v.get("back", [])  # build 058
        _need(isinstance(back, list) and len(back) <= MAX_REACH, f"{at} back must be a list of at most {MAX_REACH}")
        backs = []
        for j, r in enumerate(back):
            scene = r.get("scene")
            stop = (NO_SCENE if scene is None else _int(scene, 1, SCENES, f"{at} back scene") - 1,
                    _int(r["start"], 1, MAX_REACH_START, f"{at} back start"))
            _need(not backs or stop[1] > backs[-1][1], f"{at} back starts must rise")
            backs.append(stop)
        # Build 064: at or before it (a window starting exactly on an entry's start plays that entry)
        _need(not backs or offset + backs[-1][1] <= 0, f"{at}: the window must start at or before its last back join")
        out += struct.pack("<BBhHB", _int(v["song"], 1, 14, f"{at} song") - 1,
                           _int(v["entry"], 1, 96, f"{at} entry") - 1, offset, length,
                           (2 if v.get("unplaced", False) else 0) | (REACHING if stops else 0) | (BACK_REACHING if backs else 0))
        out += bytes(scenes)
        if stops:
            out += bytes([len(stops)])
            for scene, start in stops:
                out += struct.pack("<BH", scene, start)
        if backs:
            out += bytes([len(backs)])
            for scene, start in backs:
                out += struct.pack("<BH", scene, start)
        for n, lane in enumerate(list(lanes) + [{}] * (LANES - len(lanes))):
            out += _lane(lane, length, f"{at} lane {'abc'[n]}")
    _need(len(out) <= CAPACITY, f"{len(out)} bytes: more than the {CAPACITY} variations may keep")
    return bytes(out)


def _walk_legacy(data):
    """Version 1 or 2 bytes -> [(header bytes, {(track, line): [points bytes]})], checked as store.hpp does."""
    _need(len(data) >= 4 and data[:2] == MAGIC and data[2] in (1, 2), "not version 1 or 2 variations data")
    _need(len(data) <= CAPACITY and data[3] <= MAX_VARIATIONS, "too big")
    header = 12 if data[2] == 1 else 14
    at, out = 4, []
    for _ in range(data[3]):
        _need(at + header <= len(data), "truncated")
        h = data[at:at + header]
        at += header
        length, selection = h[4] | h[5] << 8, h[8] | h[9] << 8
        offset = struct.unpack("<h", h[2:4])[0]
        _need(h[0] < 14 and h[1] < 96 and 8 <= length <= 128 and h[6] <= 7 and selection < 256, "field out of range")
        _need(-length <= offset <= MAX_OFFSET_V11, "field out of range")
        if header == 14:
            _need(all(s < SCENES or s == NO_SCENE for s in h[11:13]), "scene out of range")
        runs = {}
        for _ in range(h[header - 1]):
            _need(at + 2 <= len(data), "truncated")
            track, line, n = data[at] & 15, data[at] >> 4, data[at + 1]
            _need(track < TRACKS and line < 4 and 1 <= n <= 32 and (track, line) not in runs, "bad point run")
            at += 2
            _need(at + 3 * n <= len(data), "truncated")
            pts = data[at:at + 3 * n]
            at += 3 * n
            last = -1
            for k in range(n):
                _need(last < pts[3 * k] < length and pts[3 * k + 2] != 0x80, "bad point")
                last = pts[3 * k]
            runs[(track, line)] = pts
        out.append((bytes(h), runs))
    _need(at == len(data), "trailing bytes")
    return out


def legacy_value(line, v):
    """A version 2 line's value as a version 3 lane's (128 as set)."""
    if line == 0:
        return (v * 128 + 127) // 255
    if line == 2:
        return 128 + (v * 127 + 127) // 255
    return v


def upgrade(data):
    """Version 1-4 bytes -> version 12 bytes (lane C empty), as store.hpp rewrites them on load. Versions 1 and 2 go
    through version 4 first (upgrade_v4: lane A the line the gesture moved (custom: the first line with
    points), lane B the next line with points (custom, on the tracks it has points on); a third and fourth
    line become a second variation on the same window (custom)); version 3's lanes take their gesture's
    own curve; then to_v6 gives each lane its one line."""
    _need(len(data) >= 4 and data[:2] == MAGIC and data[2] in (1, 2, 3, 4), "not version 1-4 variations data")
    if data[2] in (1, 2):
        data = upgrade_v4(data)
    return to_v6(data)


def upgrade_v4(data):
    """Version 1 or 2 bytes -> version 4 bytes (store.hpp upgrade)."""
    v2 = data[2] == 2
    out, count = bytearray(), 0
    for h, runs in _walk_legacy(data):
        g = h[6]
        used = {line for _, line in runs}
        natural = 2 if g == 4 else 3 if g in (5, 6) else 0
        first = natural if g else next((l for l in range(4) if l in used), 0)
        order = [first] + [l for l in range(4) if l != first and l in used]
        for c in range(0, len(order), 2):
            out += h[:6] + bytes([h[10] & 2 if v2 else 0]) + (h[11:13] if v2 else bytes([NO_SCENE, NO_SCENE]))
            for n, line in enumerate((order + [None])[c:c + 2]):
                tracks = [t for t in range(TRACKS) if line is not None and (t, line) in runs]
                if c == 0 and n == 0:
                    gesture = GESTURES_FROM_LEGACY[g]
                    target = NO_TARGET if gesture == NO_GESTURE else LINE_TARGETS[line]
                    default_target = gesture not in (0, NO_GESTURE) and target == natural_target(gesture)
                    out += bytes([gesture, h[7], target, h[8], (h[10] & 1) | (2 if default_target else 0)])
                    out += struct.pack("<bB", default_curve(gesture), len(tracks))
                elif line is None:
                    out += bytes([NO_GESTURE, 0, NO_TARGET, 0, 2, 0, 0])
                else:
                    out += bytes([0, 0, LINE_TARGETS[line], sum(1 << t for t in tracks), 0, 0, len(tracks)])
                for t in tracks:
                    pts = bytearray(runs[(t, line)])
                    for k in range(0, len(pts), 3):
                        pts[k + 1] = legacy_value(line, pts[k + 1])
                    out += bytes([t, len(pts) // 3]) + pts
            count += 1
    out = MAGIC + bytes([4, count]) + out
    _need(count <= MAX_VARIATIONS and len(out) <= CAPACITY, "too full to rewrite as version 4")
    return bytes(out)


def to_v6(data):
    """Version 3 or 4 bytes -> version 12 (store.hpp to_v6; no names, lane C empty): each lane's one line its leading track's
    (stagger.hpp: build the lowest of its tracks, break the highest, the rest the lowest), or with no line
    there the lowest track's that has one (custom: the lowest track's with points, its tracks those with
    points); build and break (unless edited) made again from the gesture,
    so their stagger puts every track's entry or exit where version 4's jumps were."""
    _need(len(data) >= 4 and data[:2] == MAGIC and data[2] in (3, 4), "not version 3 or 4 variations data")
    old = _parse_old(data, LANE_HEADER_V3 if data[2] == 3 else LANE_HEADER)
    out = bytearray(MAGIC + bytes([VERSION, len(old)]))
    for v, (head, lanes) in old:
        out += head[:6] + bytes([head[6] & 2]) + head[7:]  # no reach in old bytes, whatever their flags held
        for lane in lanes:
            g, amount, target, selection, flags, curve, runs = lane
            edited = bool(flags & 4) and g != NO_GESTURE
            lead = leading_track(g, selection)
            line = runs.get(lead) or (runs[min(runs)] if runs else [])
            if g == 0 and runs:  # custom: version 4 played the tracks with points
                selection = sum(1 << t for t in runs)
                line = runs[min(runs)]
            if g in (1, 2) and not edited and selection:
                line = entry_line(g, amount, curve, selection, v["length"])
            out += bytes([g, amount, target, selection, (flags & 3) | (4 if edited else 0)])
            out += struct.pack("<bB", curve, len(line))
            for p in line:
                out += struct.pack("<BBb", p["step"], p["value"], p.get("curve", 0))
        out += _lane({}, v["length"], "lane c")  # build 060: version 3 and 4 hold two lanes; C is empty
    _need(len(out) <= CAPACITY, "too full to rewrite as version 12")
    return bytes(out)


def _parse_old(data, lane_header):
    """Version 4 (lane_header 7) or 3 (6, each lane its gesture's own curve) bytes, checked as store.hpp
    reads them -> [({"length"}, (header bytes, [(gesture, amount, target, selection, flags, curve,
    {track: points})]))]."""
    count, at, out = data[3], 4, []
    _need(count <= MAX_VARIATIONS, "too many variations")

    def take(n):
        nonlocal at
        _need(at + n <= len(data), "truncated")
        chunk = data[at:at + n]
        at += n
        return chunk

    for _ in range(count):
        head = take(HEADER)
        song, entry, offset, length, flags = struct.unpack("<BBhHB", head[:7])
        _need(all(s < SCENES or s == NO_SCENE for s in head[7:9]), "scene out of range")
        _need(song < 14 and entry < 96 and 8 <= length <= 128 and -length <= offset <= MAX_OFFSET_V11, "field out of range")
        lanes = []
        for _ in range(2):
            if lane_header == LANE_HEADER:
                gesture, amount, target, selection, lane_flags, curve, nruns = struct.unpack("<BBBBBbB", take(7))
            else:
                gesture, amount, target, selection, lane_flags, nruns = take(6)
                curve = default_curve(gesture)
                lane_flags &= 3
            _need(gesture < len(GESTURES) or gesture == NO_GESTURE, "gesture out of range")
            _need(curve != -128, "curve out of range")
            _need(target in TARGET_NAMES or target == NO_TARGET, "target out of range")
            runs = {}
            for _ in range(nruns):
                track, n = take(2)
                _need(track < TRACKS and 1 <= n <= 32 and track not in runs, "bad point run")
                pts = []
                for _ in range(n):
                    step, value, c = struct.unpack("<BBb", take(3))
                    _need(step < length and c != -128 and (not pts or step > pts[-1]["step"]), "bad point")
                    p = {"step": step, "value": value}
                    if c:
                        p["curve"] = c
                    pts.append(p)
                runs[track] = pts
            lanes.append((gesture, amount, target, selection, lane_flags, curve, runs))
        out.append(({"length": length}, (bytes(head), lanes)))
    _need(at == len(data), "trailing bytes")
    return out


def decode(data):
    """Bytes (version 8; 7, 6, 5 the same without muted lanes, rates or names; or 1-4 as the extension rewrites them) -> JSON
    document. Raises FormatError on anything malformed."""
    _need(len(data) >= 4 and data[:2] == MAGIC, "not variations data")
    version = data[2]
    _need(version in (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12), f"format version {version}, this reads 1 to 12")
    if version < 5:
        data = upgrade(data)
    return _parse(data, named=data[2] >= 6, rated=data[2] >= 7, v8=data[2] >= 8, v9=data[2] >= 9, v10=data[2] >= 10, v12=data[2] >= 12,
                  lanes=LANES if data[2] >= 11 else LANES_V10)


def _parse(data, named=True, rated=True, v8=True, v9=True, v10=True, v12=True, lanes=LANES):
    """Version 12 bytes (v12 False: version 11's and older, within the smaller ranges: length 1024, reach and back 31, starts 4095, offset 2047) (lanes 2: version 10's and older, which hold two lanes a variation, shown as they are, lane C not made up)
    Version 10 bytes (v10 False: version 9's and older, whose backward-list flag means nothing) (v9 False: version 8's, whose lengths are up to 256 and reaches to 15 joins, starts to 1023; v8 False: version 5-7's, whose muted bit and reach flag mean nothing; the rest as below) (named False: version 5's, whose flags bit 3 means nothing; rated False: version 6's or
    5's, whose flags bits 4-5 mean nothing) -> JSON."""
    count, at, variations = data[3], 4, []
    _need(count <= MAX_VARIATIONS, "too many variations")

    def take(n):
        nonlocal at
        _need(at + n <= len(data), "truncated")
        chunk = data[at:at + n]
        at += n
        return chunk

    for _ in range(count):
        song, entry, offset, length, flags = struct.unpack("<BBhHB", take(7))
        scenes = list(take(2))
        _need(all(s < SCENES or s == NO_SCENE for s in scenes), "scene out of range")
        max_length = MAX_LENGTH if v12 else MAX_LENGTH_V11 if v9 else 256 if v8 else 128
        max_offset = MAX_OFFSET if v12 else MAX_OFFSET_V11
        _need(song < 14 and entry < 96 and 8 <= length <= max_length and -max_offset <= offset <= max_offset, "field out of range")
        if not v8:
            flags &= ~REACHING
        if not v10:
            flags &= ~BACK_REACHING
        stops = []
        if flags & REACHING:
            reach = take(1)[0]
            _need(1 <= reach <= (MAX_REACH if v12 else MAX_REACH_V11 if v9 else 15), "reach of 1..95 (31 before version 12, 15 before version 9)")
            for _ in range(reach):
                scene, start = struct.unpack("<BH", take(3))
                _need((scene < SCENES or scene == NO_SCENE) and 1 <= start <= (MAX_REACH_START if v12 else MAX_REACH_START_V11 if v9 else 1023) and (not stops or start > stops[-1][1]), "bad reach")
                stops.append((scene, start))
        backs = []
        if flags & BACK_REACHING:  # build 058
            back = take(1)[0]
            _need(1 <= back <= (MAX_REACH if v12 else MAX_REACH_V11), "back of 1..95 (31 before version 12)")
            for _ in range(back):
                scene, start = struct.unpack("<BH", take(3))
                _need((scene < SCENES or scene == NO_SCENE) and 1 <= start <= (MAX_REACH_START if v12 else MAX_REACH_START_V11) and (not backs or start > backs[-1][1]), "bad back")
                backs.append((scene, start))
        variation = {"song": song + 1, "entry": entry + 1, "offset": offset, "length": length}
        if scenes != [NO_SCENE, NO_SCENE]:
            variation["scenes"] = [None if s == NO_SCENE else s + 1 for s in scenes]
        if flags & 2:
            variation["unplaced"] = True
        if stops:
            variation["reach"] = [{"scene": None if sc == NO_SCENE else sc + 1, "start": st} for sc, st in stops]
        if backs:
            variation["back"] = [{"scene": None if sc == NO_SCENE else sc + 1, "start": st} for sc, st in backs]
        found = []
        for _ in range(lanes):
            gesture, amount, target, selection, lane_flags, lane_curve = struct.unpack("<BBBBBb", take(6))
            _need(gesture < len(GESTURES) or gesture == NO_GESTURE, "gesture out of range")
            _need(lane_curve != -128, "lane_curve out of range")
            _need(target in TARGET_NAMES or target == NO_TARGET, "target out of range")
            if not v8:
                lane_flags &= ~MUTED
            if not rated:
                lane_flags &= ~RATE_MASK
            rate = RATES[lane_flags >> RATE_SHIFT & 3]
            control = None
            if not named:
                lane_flags &= ~NAMED
            if lane_flags & NAMED:
                _need(target < ENGINE_WORDS, "a control's name on a target that is no engine control")
                name_length = take(1)[0]
                _need(1 <= name_length <= NAME_CHARS, "a control's name of 1..15 characters")
                raw = take(name_length)
                control = raw.decode("ascii", "replace")
                _need(_name_ok(control), "a control's name: lowercase characters")
            n = take(1)[0]
            _need(n <= 32, "a line has at most 32 points")
            points = []
            for _ in range(n):
                step, value, curve = struct.unpack("<BBb", take(3))
                _need(step < line_length(length) and curve != -128 and (not points or step > points[-1]["step"]), "bad point")
                point = {"step": step, "value": value}
                if curve:
                    point["curve"] = curve
                points.append(point)
            found.append({
                "gesture": "none" if gesture == NO_GESTURE else GESTURES[gesture], "amount": amount,
                "target": TARGET_NAMES.get(target), "tracks": [t + 1 for t in range(TRACKS) if selection >> t & 1],
                "default_tracks": bool(lane_flags & 1), "default_target": bool(lane_flags & 2), "curve": lane_curve,
                "line": points,
            })
            if lane_flags & 4 and gesture != NO_GESTURE:
                found[-1]["edited"] = True
            if rate != 1:
                found[-1]["rate"] = rate
            if lane_flags & MUTED:
                found[-1]["muted"] = True
            if control:
                found[-1]["control"] = control
        variation["lanes"] = found
        variations.append(variation)
    _need(at == len(data), "trailing bytes")
    return {"variations": variations}


# Version 1 and 2, as older builds wrote them (tests of the upgrade).


def _legacy_turn_step(r, n, amount, length):
    """Where the r-th of n tracks enters (build) or leaves (break), in steps."""
    num = length * (n * 255 + (2 * r + 1) * amount) - length * n * amount
    den = 2 * n * 255
    step = (num + den // 2) // den
    grid = 4 if length >= 16 else 1
    step = (step + grid // 2) // grid * grid
    return min(max(step, grid), length - grid)


def _legacy_drop_gap(amount, length):
    return min(4 + (amount * 12 + 127) // 255, length - 1)


def legacy_gesture_lines(gesture, amount, length, tracks, rest=()):
    """A gesture's point runs, as the page makes them: [{"track", "line", "points"}],
    in track then line order (as the extension stores them). Tracks from 1."""
    selected = sorted(set(tracks))
    rest = sorted(set(rest) - set(selected))
    runs = {}

    def point(step, value, curve=0):
        p = {"step": step, "value": value}
        if curve:
            p["curve"] = curve
        return p

    def ramp(track, line, start, end, curve):
        if start != end:
            runs[(track, line)] = [point(0, start), point(length - 1, end, curve)]

    def jump(track, line, at, start, end):
        pts = [point(0, start)]
        if at > 1:
            pts.append(point(at - 1, start))
        pts.append(point(at, end, SUDDEN))
        runs[(track, line)] = pts

    n, a = len(selected), amount
    if gesture == "build":
        busier = 128 + (a * 127 + 127) // 255
        for r, t in enumerate(selected):
            jump(t, "level", _legacy_turn_step(r, n, a, length), 0, 255)
        for t in selected + rest:
            ramp(t, "notes", 128, busier, SLOW_START)
    elif gesture == "break":
        thinner = 128 - (a * 96 + 127) // 255
        for r, t in enumerate(reversed(selected)):
            jump(t, "level", _legacy_turn_step(r, n, a, length), 255, 0)
        for t in rest:
            ramp(t, "notes", 128, thinner, SLOW_START)
    elif gesture == "drop":
        cut = length - _legacy_drop_gap(a, length)
        for t in selected:
            jump(t, "level", cut, 255, 0)
    elif gesture == "swell":
        for t in selected:
            ramp(t, "send", 0, a, SWELL_CURVE)
    elif gesture in ("filter rise", "filter fall"):
        up = gesture == "filter rise"
        to = 128 + (a * 127 + 127) // 255 if up else 128 - (a * 128 + 127) // 255
        for t in selected:
            ramp(t, "filter", 128, to, SLOW_START if up else FAST_START)
    return [{"track": t, "line": line, "points": runs[(t, line)]}
            for t, line in sorted(runs, key=lambda k: (k[0], LINES.index(k[1])))]


def encode_legacy(doc, version=2):
    """Version 1 or 2 JSON (one gesture, "lines" of level/notes/send/filter) -> version 1 or 2 bytes:
    the data older builds saved, for tests of the upgrade."""
    _need(version in (1, 2), "version 1 or 2")
    _need(isinstance(doc, dict) and isinstance(doc.get("variations"), list), "expected {\"variations\": [...]}")
    variations = doc["variations"]
    _need(len(variations) <= MAX_VARIATIONS, f"at most {MAX_VARIATIONS} variations")
    out = bytearray(MAGIC + bytes([version, len(variations)]))
    for i, v in enumerate(variations):
        at = f"variation {i + 1}"
        length = _int(v["length"], 8, 128, f"{at} length")
        offset = _int(v["offset"], -length, MAX_OFFSET_V11, f"{at} offset")  # a window before the join ends on it at the latest
        _need(v["gesture"] in LEGACY_GESTURES, f"{at} gesture must be one of {LEGACY_GESTURES}")
        tracks = v["tracks"]
        _need(isinstance(tracks, list), f"{at} tracks must be a list")
        selection = 0
        for t in tracks:
            selection |= 1 << (_int(t, 1, 8, f"{at} track") - 1)
        if "lines" in v or v["gesture"] == "custom":
            lines = v.get("lines", [])
        else:
            lines = legacy_gesture_lines(v["gesture"], _int(v["amount"], 0, 255, f"{at} amount"), length, tracks,
                                  [_int(t, 1, 8, f"{at} rest track") for t in v.get("rest", [])])
        _need(len(lines) <= 255, f"{at}: at most 255 point runs")
        scenes = v.get("scenes", [None, None])
        _need(isinstance(scenes, list) and len(scenes) == 2, f"{at} scenes must be [before, after]")
        scenes = [NO_SCENE if s is None else _int(s, 1, SCENES, f"{at} scene") - 1 for s in scenes]
        flags = (1 if v.get("default_tracks", False) else 0) | (2 if v.get("unplaced", False) else 0)
        if version == 1:
            flags &= 1
        out += struct.pack(
            "<BBhHBBHB",
            _int(v["song"], 1, 14, f"{at} song") - 1,
            _int(v["entry"], 1, 96, f"{at} entry") - 1,
            offset,
            length,
            LEGACY_GESTURES.index(v["gesture"]),
            _int(v["amount"], 0, 255, f"{at} amount"),
            selection,
            flags,
        )
        if version == 2:
            out += bytes(scenes)
        out += bytes([len(lines)])
        seen = set()
        for run in lines:
            track = _int(run["track"], 1, 8, f"{at} line track") - 1
            _need(run["line"] in LINES, f"{at} line must be one of {LINES}")
            line = LINES.index(run["line"])
            _need((track, line) not in seen, f"{at}: track {track + 1} {run['line']} given twice")
            seen.add((track, line))
            points = run["points"]
            _need(1 <= len(points) <= 32, f"{at}: a line has 1..32 points")
            out += bytes([track | line << 4, len(points)])
            last = -1
            for p in points:
                step = _int(p["step"], 0, length - 1, f"{at} point step")
                _need(step > last, f"{at}: points must be in step order, one a step")
                last = step
                out += struct.pack(
                    "<BBb", step, _int(p["value"], 0, 255, f"{at} point value"),
                    _int(p.get("curve", 0), -127, 127, f"{at} point curve"))
    _need(len(out) <= CAPACITY, f"{len(out)} bytes: more than the {CAPACITY} variations may keep")
    return bytes(out)


def main(argv):
    if len(argv) != 4 or argv[1] not in ("encode", "decode"):
        print(__doc__.split("\n\n")[0])
        print("usage: variations.py encode in.json out.bin | decode in.bin out.json")
        return 2
    try:
        if argv[1] == "encode":
            with open(argv[2]) as f:
                data = encode(json.load(f))
            with open(argv[3], "wb") as f:
                f.write(data)
            print(f"{len(data)} bytes")
        else:
            with open(argv[2], "rb") as f:
                doc = decode(f.read())
            with open(argv[3], "w") as f:
                json.dump(doc, f, indent=2)
                f.write("\n")
    except (FormatError, KeyError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
