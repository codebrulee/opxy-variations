"""Host tests for tools/variations.py: python -m unittest discover -s tests"""
import json
import os
import re
import subprocess
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "tools"))
import variations as v  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
# The SDK beside this repository (open-xy-vdev, or open-xy), for its track targets.
SDK_HEADERS = [os.path.join(ROOT, "..", d, "sdk", "include", "opxy", "extension.hpp") for d in ("open-xy-vdev", "open-xy")]


def cpp_array(name, source):
    """The bytes of `static ... name[] = {...};` in tests/<source>."""
    with open(os.path.join(HERE, source)) as f:
        m = re.search(re.escape(name) + r"\[\] = \{([0-9, ]*)\};", f.read())
    return bytes(int(b) for b in m.group(1).split(", ")) if m else None


EMPTY_LANE = bytes([255, 128, 255, 0, 2, 0, 0])  # lane C as an empty lane is written (build 060)


def _ends(data, lanes):
    """Where each variation of `data` ends, its lanes `lanes` many (build 060), flags read as the bytes are."""
    at, ends = 4, []
    for _ in range(data[3]):
        flags = data[at + 6]
        at += 9
        for bit in (4, 8):
            if flags & bit:
                at += 1 + 3 * data[at]
        for _ in range(lanes):
            named = data[at + 4] & 8
            at += 6
            if named:
                at += 1 + data[at]
            at += 1 + 3 * data[at]
        ends.append(at)
    return ends


def as_old(data, version):
    """Current (version 12) bytes whose lane C is empty -> the bytes version `version` (10 or below) would have been:
    two lanes a variation. Version 11 (build 063: the same bytes, within the smaller ranges): only the number changes."""
    if version == 11:
        out = bytearray(data)
        out[2] = 11
        return out
    data, out, last = bytes(data), bytearray(), 0
    for end in _ends(data, 3):
        assert data[end - 7:end] == EMPTY_LANE, "lane C is not empty"
        out += data[last:end - 7]
        last = end
    out += data[last:]
    out[2] = version
    return out


def as_current(data):
    """Version 10 or older bytes (two lanes a variation) -> current (version 12) bytes with lane C empty, as the store loads them."""
    data, out, last = bytes(data), bytearray(), 0
    for end in _ends(data, 2):
        out += data[last:end] + EMPTY_LANE
        last = end
    out += data[last:]
    out[2] = 12
    return out


def fixture(name):
    with open(os.path.join(HERE, "fixtures", name)) as f:
        return json.load(f)


BUILD = fixture("build.json")


class Format(unittest.TestCase):
    def test_round_trip(self):
        data = v.encode(BUILD)
        self.assertEqual(data[:4], b"VR\x0c\x02")
        first = 9 + 2 * 7 + 6 + 6  # version 5: one line a lane
        self.assertEqual(len(data), 4 + first + 9 + 2 * 7 + 3 + 2 * 7)  # build 060: lane C empty, 7 bytes a variation
        doc = v.decode(data)
        self.assertEqual(v.encode(doc), data)
        a, b, c = doc["variations"][0]["lanes"]
        self.assertEqual((c["gesture"], c["target"], c["tracks"], c["line"]), ("none", None, [], []))  # build 060: lane C
        self.assertEqual((a["gesture"], a["target"], a["tracks"], a["default_tracks"], a["default_target"]),
                         ("break", "volume", [1, 2, 3], True, True))
        self.assertEqual((b["gesture"], b["target"], b["tracks"], b["default_target"]), ("rise", "release", [5], False))
        empty = doc["variations"][1]["lanes"][1]
        self.assertEqual((empty["gesture"], empty["target"], empty["tracks"], empty["line"]), ("none", None, [], []))

    def test_edited(self):
        # The shape page's edit of a gesture (lane flags bit 2): its own line kept, round trip.
        lane = dict(BUILD["variations"][0]["lanes"][0], edited=True,
                    line=[{"step": 0, "value": 40}, {"step": 9, "value": 200, "curve": -30}])
        doc = {"variations": [dict(BUILD["variations"][0], lanes=[lane])]}
        data = v.encode(doc)
        self.assertEqual(data[17], 1 | 2 | 4)
        back = v.decode(data)["variations"][0]["lanes"][0]
        self.assertTrue(back["edited"])
        self.assertEqual(back["line"], lane["line"])
        self.assertEqual(v.encode(v.decode(data)), data)
        self.assertNotIn("edited", v.decode(v.encode(BUILD))["variations"][0]["lanes"][0])

    def test_empty(self):
        self.assertEqual(v.decode(v.encode({"variations": []})), {"variations": []})

    def test_fields_on_the_wire(self):
        data = v.encode(BUILD)
        self.assertEqual(data[4], 1)  # song 2
        self.assertEqual(data[5], 2)  # entry 3
        self.assertEqual(int.from_bytes(data[6:8], "little", signed=True), -32)
        self.assertEqual(data[8:10], bytes([32, 0]))
        self.assertEqual(data[10], 0)  # flags: placed
        self.assertEqual(data[11:13], b"\xff\xff")  # scenes not known
        self.assertEqual(data[13:20], bytes([2, 200, 15, 0b111, 1 | 2, 0, 2]))  # lane a: break, volume, tracks 1-3, straight, 2 points
        self.assertEqual(data[20:23], bytes([0, 128, 0]))  # its first point
        b = 20 + 6
        self.assertEqual(data[b:b + 7], bytes([4, 128, 11, 0x10, 2 << 4, 60, 2]))  # lane b: rise, release, track 5, rate 4x (flags bits 4-5), slow 60, 2 points
        self.assertEqual(data[b + 7:b + 10], bytes([0, 128, 0]))

    def test_scenes_and_unplaced(self):
        doc = {"variations": [dict(BUILD["variations"][0], scenes=[1, 2]),
                              dict(BUILD["variations"][0], scenes=[None, 99], unplaced=True)]}
        data = v.encode(doc)
        self.assertEqual(data[11:13], bytes([0, 1]))
        self.assertEqual(v.decode(data)["variations"][1]["scenes"], [None, 99])
        self.assertTrue(v.decode(data)["variations"][1]["unplaced"])
        for bad in ([0, 1], [1, 100], [1], "1,2"):
            with self.assertRaises(v.FormatError):
                v.encode({"variations": [dict(BUILD["variations"][0], scenes=bad)]})

    def test_targets_by_name_or_word(self):
        lane = {"gesture": "rise", "amount": 10, "tracks": [1]}
        for target, word in (("cutoff", 16), ("lfo speed", 24), ("param 3", 2), ("m1 3", 2), ("notes", 0x40), (40, 40), (None, 0xff)):
            data = v.encode({"variations": [dict(BUILD["variations"][0], lanes=[dict(lane, target=target)])]})
            self.assertEqual(data[15], word)
        # Left out: the gesture's own, following it.
        data = v.encode({"variations": [dict(BUILD["variations"][0], lanes=[lane, {"gesture": "drop", "tracks": [2]}])]})
        self.assertEqual((data[15], data[17] & 2), (16, 2))
        lane_b = 13 + 7 + 6
        self.assertEqual((data[lane_b + 2], data[lane_b + 4] & 2), (15, 2))

    def test_offsets(self):
        # Version 5 (build 024): a window may lie wholly in the section before the join, as far back as 128 bars;
        # build 063 (version 12): as far as 8191 steps either way (2047 until then).
        offsets = (-8191, -4095, -2048, -2047, -200, -33, -32, -1, 0, 2047, 2048, 4095, 8191)
        doc = {"variations": [dict(BUILD["variations"][0], offset=o) for o in offsets]}
        self.assertEqual([x["offset"] for x in v.decode(v.encode(doc))["variations"]], list(offsets))
        for bad in (-8192, 8192):
            with self.assertRaises(v.FormatError):
                v.encode({"variations": [dict(BUILD["variations"][0], offset=bad)]})

    def test_refusals(self):
        base = BUILD["variations"][0]
        lane = base["lanes"][0]
        bad_lanes = [
            dict(lane, gesture="wobble"), dict(lane, amount=256), dict(lane, tracks=[9]),
            dict(lane, target="mix level"), dict(lane, target=41), dict(lane, target=12), dict(lane, target=26),
            dict(lane, line=[{"step": 32, "value": 0}]),
            dict(lane, line=[{"step": 3, "value": 0}, {"step": 3, "value": 1}]),
            dict(lane, line=[{"step": 0, "value": 0, "curve": -128}]),
            dict(lane, line=[{"step": s, "value": 0} for s in range(33)]),
            dict(lane, lines=[{"track": 1, "points": [{"step": 0, "value": 0}]}]),  # version 4's form
        ]
        bad = [dict(base, song=15), dict(base, entry=0), dict(base, length=7), dict(base, length=4097),
               dict(base, lanes=[]), dict(base, lanes=[lane] * 4)]  # build 060: three lanes are fine now + [dict(base, lanes=[l]) for l in bad_lanes]
        for doc in bad:
            with self.assertRaises(v.FormatError, msg=doc):
                v.encode({"variations": [doc]})

    def test_capacity(self):
        # Version 5: the largest variation (both lanes 32 points) was 215 bytes (version 4: 1591); build 060
        # (version 11): the empty lane C makes it 222, and three full lanes 9 + 3 * 103 = 318.
        lane = {"gesture": "custom", "target": "volume", "tracks": list(range(1, 9)),
                "line": [{"step": s, "value": s} for s in range(32)]}
        two = dict(BUILD["variations"][0], length=128, lanes=[lane, dict(lane, target="cutoff")])
        self.assertEqual(len(v.encode({"variations": [two]})), 4 + 222)
        big = dict(two, lanes=[lane, dict(lane, target="cutoff"), dict(lane, target="fx i send")])
        self.assertEqual(len(v.encode({"variations": [big]})), 4 + 318)
        self.assertLessEqual(len(v.encode({"variations": [big] * 25})), v.CAPACITY)
        with self.assertRaises(v.FormatError):
            v.encode({"variations": [big] * 26})
        small = dict(BUILD["variations"][0], lanes=[{"gesture": "none"}])
        v.encode({"variations": [small] * 64})
        with self.assertRaises(v.FormatError):
            v.encode({"variations": [small] * 65})

    def test_decode_refuses_damage(self):
        data = v.encode(BUILD)
        for broken in (data[:-1], data + b"\0", b"XX" + data[2:], data[:2] + b"\x0d" + data[3:]):
            with self.assertRaises(v.FormatError):
                v.decode(broken)
        for at, value in ((13, 19), (15, 41), (15, 12), (18, 0x80), (19, 33), (20, 32)):
            broken = bytearray(data)
            broken[at] = value
            with self.assertRaises(v.FormatError):
                v.decode(bytes(broken))


class Curves(unittest.TestCase):
    def test_defaults_per_gesture(self):
        self.assertEqual({g: v.default_curve(g) for g in v.GESTURES + ["none"]},
                         {"custom": 0, "build": 0, "break": 0, "drop": 0, "rise": 60, "fall": -60, "swell": -40,
                          "dip": -40, "fade in": 60, "fade out": -60, "random": 0, "wander": 0,
                          "cut in": 0, "gate": 0, "stutter": 0, "stutter in": 0, "spring": 0,
                          "stutter out": 0, "hold": 0, "none": 0})
        lane = {"gesture": "rise", "amount": 128, "tracks": [1]}
        doc = v.decode(v.encode({"variations": [dict(BUILD["variations"][0], lanes=[lane])]}))
        self.assertEqual(doc["variations"][0]["lanes"][0]["curve"], 60)
        self.assertEqual(doc["variations"][0]["lanes"][1]["curve"], 0)  # an empty lane b

    def test_round_trip_and_range(self):
        for curve in (-127, -60, 0, 33, 127):
            lane = {"gesture": "swell", "amount": 200, "tracks": [2], "curve": curve}
            data = v.encode({"variations": [dict(BUILD["variations"][0], lanes=[lane])]})
            self.assertEqual(data[18], curve & 0xff)
            back = v.decode(data)
            self.assertEqual(back["variations"][0]["lanes"][0]["curve"], curve)
            self.assertEqual(v.encode(back), data)
        for bad in (-128, 128, "slow"):
            with self.assertRaises(v.FormatError):
                v.encode({"variations": [dict(BUILD["variations"][0], lanes=[{"gesture": "rise", "curve": bad}])]})

    def test_bend_applied(self):
        # rise and fall: the ramp's curve; swell and dip: out by it, back by its mirror.
        self.assertEqual(v.lane_line("rise", 255, 32, [1], 100)[-1]["curve"], 100)
        self.assertEqual(v.lane_line("fall", 255, 32, [1], -20)[-1]["curve"], -20)
        swell = v.lane_line("swell", 255, 32, [1], 50)
        self.assertEqual([p.get("curve", 0) for p in swell], [0, 50, -50])
        # build and break: a slow start brings the entries later, a fast start earlier (the line and its stagger).
        steps = lambda c: [v.lane_line("build", 255, 128, [1, 2, 3, 4], c)[-1]["step"] + v.stagger(1, 255, c, 0b1111, t, 128)
                           for t in range(4)]
        straight, slow, fast = steps(0), steps(100), steps(-100)
        self.assertTrue(all(s >= m for s, m in zip(slow, straight)) and sum(slow) > sum(straight))
        self.assertTrue(all(f <= m for f, m in zip(fast, straight)) and sum(fast) < sum(straight))
        self.assertEqual(slow, sorted(slow))
        # drop has none: the same cut whatever the curve.
        self.assertEqual(v.lane_line("drop", 128, 32, [1], 100), v.lane_line("drop", 128, 32, [1], -100))

    def test_bend_matches_line_hpp(self):
        # line.hpp's bend against pow, as tests/line.cpp holds it.
        for curve in (-127, -60, -1, 1, 40, 127):
            for f in (1, 100, 16384, 32768, 50000, 65535):
                want = (f / 65536) ** (2 ** (2 * curve / 127)) * 65536
                self.assertLess(abs(v.bend(f, curve) - want), 65536 * 0.01, (curve, f))

    def test_version_3_and_4_upgrade(self):
        # store.cpp's version 3 and 4 copies of build.json as version 4 had it (lane a a break with its own
        # points on tracks 1 and 2): version 3's lanes take their gesture's own curve; both become version 5
        # alike (store.cpp: "v5"): the break made again from the gesture, lane b track 5's line.
        old, v4 = cpp_array("v3", "store.cpp"), cpp_array("v4", "store.cpp")
        self.assertEqual((old[:3], v4[:3]), (b"VR\x03", b"VR\x04"))
        self.assertEqual(v.upgrade(old), v.upgrade(v4))
        self.assertEqual(cpp_array("v6", "store.cpp"), v.upgrade(v4))
        self.assertEqual(v.encode(v.decode(old)), v.upgrade(old))
        lanes = v.decode(old)["variations"][0]["lanes"]
        self.assertEqual([l["curve"] for l in lanes], [0, 60, 0])  # build 060: lane C empty
        self.assertEqual(lanes[0]["line"], v.entry_line(2, 200, 0, 0b111, 32))
        self.assertEqual(lanes[1]["line"], BUILD["variations"][0]["lanes"][1]["line"])
        with self.assertRaises(v.FormatError):
            v.decode(old[:-1])


class Rise(unittest.TestCase):
    def test_rise_arrives_fall_leaves(self):
        # Build 036: rise leaves as set, up by the amount; fall leaves it, down by the amount.
        for amount, low, high in ((255, 0, 255), (128, 64, 192), (0, None, None)):
            rise = v.lane_line("rise", amount, 32, [1])
            fall = v.lane_line("fall", amount, 32, [1])
            if low is None:
                self.assertEqual((rise, fall), ([], []))
                continue
            self.assertEqual([p["value"] for p in rise], [128, high])
            self.assertEqual([p["value"] for p in fall], [128, low])


class Targets(unittest.TestCase):
    def test_the_sdks_table(self):
        # TARGETS is the SDK's track_targets (opxy/extension.hpp), by label, the engine's by position.
        header = next((p for p in SDK_HEADERS if os.path.exists(p)), None)
        if not header:
            self.skipTest("no SDK beside this repository")
        with open(header) as f:
            text = f.read()
        if "track_targets" not in text:
            self.skipTest("an SDK without track targets")
        rows = re.findall(r'\{(\d+), track_page::\w+, (\d+), (-?1), track_smoothing::\w+, track_reach::\w+, '
                          r'(nullptr|"[^"]*")\}', text)
        sdk = {int(w): (f"param {int(p)}" if l == "nullptr" else l.strip('"')) for w, p, _, l in rows}
        ours = {w: n for n, w in v.TARGETS.items() if n != "notes"}
        self.assertEqual(ours, sdk)
        # And store.hpp's target_ok takes the same (it asks the SDK).


class Controls(unittest.TestCase):
    """Version 6 (build 025): an engine control by name ("control"), after the lane's curve."""

    def lane(self, **kw):
        return dict({"gesture": "rise", "amount": 10, "tracks": [1, 3], "target": "param 2", "control": "tone"}, **kw)

    def test_round_trip(self):
        doc = {"variations": [dict(BUILD["variations"][0], lanes=[self.lane()])]}
        data = v.encode(doc)
        self.assertEqual(data[2], 12)
        lane = 13
        self.assertEqual(data[lane + 2], 1)  # the slot it was chosen in
        self.assertEqual(data[lane + 4] & v.NAMED, v.NAMED)
        self.assertEqual(data[lane + 6:lane + 11], bytes([4]) + b"tone")
        back = v.decode(data)["variations"][0]["lanes"][0]
        self.assertEqual((back["target"], back["control"]), ("param 2", "tone"))
        self.assertEqual(v.encode(v.decode(data)), data)
        # A slot alone: no name, no bytes.
        plain = v.encode({"variations": [dict(BUILD["variations"][0], lanes=[self.lane(control=None)])]})
        self.assertEqual(len(plain), len(data) - 5)
        self.assertNotIn("control", v.decode(plain)["variations"][0]["lanes"][0])

    def test_refused(self):
        for bad in ({"control": ""}, {"control": "Tone"}, {"control": "a sixteen chars!"}, {"target": "cutoff"}, {"control": 3}):
            with self.assertRaises(v.FormatError):
                v.encode({"variations": [dict(BUILD["variations"][0], lanes=[self.lane(**bad)])]})
        data = bytearray(v.encode({"variations": [dict(BUILD["variations"][0], lanes=[self.lane()])]}))
        for at, value in ((13 + 6, 0), (13 + 6, 16), (13 + 7, ord("T")), (13 + 2, 16)):
            broken = bytearray(data)
            broken[at] = value
            with self.assertRaises(v.FormatError):
                v.decode(bytes(broken))
        for cut in range(4, len(data)):
            with self.assertRaises(v.FormatError):
                v.decode(bytes(data[:cut]))

    def test_version_5(self):
        # Version 5 bytes are version 6's without names; flags bit 3 meant nothing then.
        data = bytearray(v.encode({"variations": [dict(BUILD["variations"][0], lanes=[self.lane(control=None)])]}))
        data = as_old(data, 5)
        data[13 + 4] |= v.NAMED
        lane = v.decode(bytes(data))["variations"][0]["lanes"][0]
        self.assertEqual(lane["target"], "param 2")
        self.assertNotIn("control", lane)
        data[13 + 4] &= ~v.NAMED
        self.assertEqual(v.encode(v.decode(bytes(data))), bytes(as_current(data)))


class Reach(unittest.TestCase):
    """Version 8 (build 047): muted lanes, reach lists, 256-step windows."""

    def doc(self, **kw):
        v0 = dict(BUILD["variations"][0], length=256, offset=0, **kw)
        v0["lanes"] = [dict(l) for l in v0["lanes"]]
        return {"variations": [v0]}

    def test_round_trip(self):
        reach = [{"scene": 4, "start": 64}, {"scene": None, "start": 200}]
        doc = self.doc(reach=reach)
        doc["variations"][0]["lanes"][1]["muted"] = True
        data = v.encode(doc)
        self.assertEqual(data[4 + 6] & 4, 4)
        self.assertEqual(data[4 + 9:4 + 9 + 7], bytes([2, 3, 64, 0, 255, 200, 0]))  # count, then {scene, u16 start}
        back = v.decode(data)["variations"][0]
        self.assertEqual(back["length"], 256)
        self.assertEqual(back["reach"], reach)
        self.assertNotIn("muted", back["lanes"][0])
        self.assertTrue(back["lanes"][1]["muted"])
        self.assertEqual(v.encode(v.decode(data)), data)
        plain = v.encode(self.doc())
        self.assertEqual(plain[4 + 6] & 4, 0)
        self.assertNotIn("reach", v.decode(plain)["variations"][0])

    def test_refused(self):
        for bad in ([{"scene": 4, "start": 0}], [{"scene": 4, "start": 64}, {"scene": 5, "start": 64}],
                    [{"scene": 100, "start": 64}], [{"scene": 4, "start": 12287}], [{"scene": 4, "start": 300}],
                    [{"scene": 4, "start": 1}] * 2, [{"scene": 4, "start": s + 1} for s in range(96)]):
            with self.assertRaises(v.FormatError, msg=bad):
                v.encode(self.doc(reach=bad))
        with self.assertRaises(v.FormatError):
            v.encode(dict(self.doc(), variations=[dict(self.doc()["variations"][0], length=4097)]))
        data = bytearray(v.encode(self.doc(reach=[{"scene": 4, "start": 64}])))
        for at, value in ((4 + 9, 0), (4 + 9, 16), (4 + 10, 99), (4 + 11, 0)):
            broken = bytearray(data)
            broken[at] = value
            if at == 4 + 11:
                broken[at + 1] = 0
            with self.assertRaises(v.FormatError, msg=(at, value)):
                v.decode(bytes(broken))

    def test_version_7(self):
        # Version 7 bytes are version 8's with no reach and no muted lane: both bits meant nothing then.
        data = bytearray(v.encode(BUILD))
        data = as_old(data, 7)
        data[4 + 6] |= 4
        data[13 + 4] |= 0x40
        back = v.decode(bytes(data))["variations"][0]
        self.assertNotIn("reach", back)
        self.assertNotIn("muted", back["lanes"][0])
        data[4 + 6] &= ~4
        data[13 + 4] &= ~0x40
        self.assertEqual(v.encode(v.decode(bytes(data))), bytes(as_current(data)))


class Rates(unittest.TestCase):
    """Version 7 (build 041): a lane's rate, flags bits 4-5 (log2)."""

    def doc(self, *rates):
        lanes = [{"gesture": "rise", "amount": 10, "tracks": [1], "line": [{"step": 0, "value": 128}], **({} if r == 1 else {"rate": r})}
                 for r in rates]
        return {"variations": [dict(BUILD["variations"][0], lanes=lanes)]}

    def test_round_trip(self):
        for rate in v.RATES:
            data = v.encode(self.doc(rate, 1))
            self.assertEqual(data[13 + 4] >> 4 & 3, [1, 2, 4, 8].index(rate))
            self.assertEqual(data[13 + 4] & 0xf, 2)  # the other flags as they were (default target)
            back = v.decode(data)["variations"][0]["lanes"]
            self.assertEqual(back[0].get("rate", 1), rate)
            self.assertNotIn("rate", back[1])
            self.assertEqual(v.encode(v.decode(data)), data)
        # Both lanes their own.
        back = v.decode(v.encode(self.doc(2, 8)))["variations"][0]["lanes"]
        self.assertEqual((back[0]["rate"], back[1]["rate"]), (2, 8))

    def test_refused(self):
        for bad in (0, 3, 16, "2", True, 1.5):
            doc = self.doc(1)
            doc["variations"][0]["lanes"][0]["rate"] = bad
            with self.assertRaises(v.FormatError, msg=bad):
                v.encode(doc)

    def test_version_6(self):
        # Version 6 bytes are version 8's with every rate 1: flags bits 4-5 meant nothing then.
        data = bytearray(v.encode(self.doc(1, 1)))
        data = as_old(data, 6)
        data[13 + 4] |= 0x30
        lanes = v.decode(bytes(data))["variations"][0]["lanes"]
        self.assertNotIn("rate", lanes[0])
        data[13 + 4] &= ~0x30
        self.assertEqual(v.encode(v.decode(bytes(data))), bytes(as_current(data)))
        # tests/store.cpp's was6: build.json as version 6 saved it.
        was6 = cpp_array("was6", "store.cpp")
        self.assertEqual(was6[2], 6)
        plain = json.loads(json.dumps(BUILD))
        del plain["variations"][0]["lanes"][1]["rate"]
        self.assertEqual(v.encode(plain), bytes(as_current(was6)))
        self.assertEqual(v.encode(v.decode(was6)), bytes(as_current(was6)))


class Upgrade(unittest.TestCase):
    def test_version_2_and_1(self):
        legacy = fixture("legacy.json")
        for version in (1, 2):
            old = v.encode_legacy(legacy, version)
            self.assertEqual(old[:3], b"VR" + bytes([version]))
            doc = v.decode(old)
            a, b, c = doc["variations"][0]["lanes"]
            self.assertEqual((c["gesture"], c["target"], c["line"]), ("none", None, []))  # build 060: old data has two lanes
            # A build: lane a on the volume (0..255 as 0..128), its notes line lane b, custom.
            self.assertEqual((a["gesture"], a["target"], a["tracks"], a["default_tracks"], a["default_target"]),
                             ("build", "volume", [1, 2, 3], True, True))
            self.assertEqual((a["line"][0]["value"], a["line"][-1]["value"]), (0, 128))  # made again: the stagger
            self.assertEqual((b["gesture"], b["target"], b["tracks"], b["default_target"]), ("custom", "notes", [2], False))
            filt = doc["variations"][1]["lanes"]
            self.assertEqual((filt[0]["target"], filt[1]["gesture"]), ("cutoff", "none"))
            self.assertEqual(v.encode(doc), v.upgrade(old))  # decoded as the extension rewrites it
        # store.cpp holds store.hpp's rewrite to the same bytes.
        self.assertEqual(cpp_array("legacy2", "store.cpp"), v.encode_legacy(legacy, 2))
        self.assertEqual(cpp_array("legacy1", "store.cpp"), v.encode_legacy(legacy, 1))
        self.assertEqual(cpp_array("upgraded", "store.cpp"), v.upgrade(v.encode_legacy(legacy, 2)))
        self.assertEqual(cpp_array("old", "repin.cpp"), v.encode_legacy(legacy, 1))

    def test_gestures_and_values(self):
        lane = lambda doc: v.decode(v.encode_legacy(doc, 2))["variations"][0]["lanes"][0]
        base = {"song": 1, "entry": 2, "offset": -32, "length": 32, "amount": 255, "tracks": [1], "default_tracks": True}
        swell = lane({"variations": [dict(base, gesture="swell")]})
        self.assertEqual((swell["gesture"], swell["target"], swell["default_target"]), ("rise", "fx i send", False))
        self.assertEqual([p["value"] for p in swell["line"]], [128, 255])  # send 0..255 as 128..255
        for old, new in (("filter rise", "rise"), ("filter fall", "fall")):
            got = lane({"variations": [dict(base, gesture=old)]})
            self.assertEqual((got["gesture"], got["target"], got["default_target"]), (new, "cutoff", True))
        clear = lane({"variations": [dict(base, gesture="clear")]})
        self.assertEqual((clear["gesture"], clear["target"], clear["line"]), ("none", None, []))

    def test_vartest_keeps_every_line(self):
        # vartest's second variation had three lines: the third becomes a variation of its own.
        with open(os.path.join(ROOT, "examples", "first-test.json")) as f:
            old = v.encode_legacy(json.load(f), 1)
        doc = v.decode(old)
        # (Version 5: a custom lane's tracks are those that had points, all following the lowest's line.)
        lanes = [[(l["target"], l["tracks"] if l["line"] else []) for l in x["lanes"]] for x in doc["variations"]]
        self.assertEqual(lanes, [[("notes", [1]), ("fx i send", [2]), (None, [])],
                                 [("volume", [4]), ("notes", [4]), (None, [])],
                                 [("cutoff", [3]), (None, []), (None, [])]])  # build 060: lane C empty throughout
        self.assertEqual([x["offset"] for x in doc["variations"]], [-32, 32, 32])

    def test_too_full(self):
        # 64 version 1 variations with two lines each: 128 as version 3, too many.
        base = {"song": 1, "entry": 2, "offset": -32, "length": 32, "gesture": "custom", "amount": 0, "tracks": [1],
                "lines": [{"track": 1, "line": l, "points": [{"step": 0, "value": 9}]} for l in ("level", "send", "filter")]}
        old = v.encode_legacy({"variations": [base] * 64}, 2)
        with self.assertRaises(v.FormatError):
            v.upgrade(old)

    def test_vartest_copies_are_current(self):
        # vartest as build 003 saved it: version 1, the upgrade's test case.
        with open(os.path.join(ROOT, "examples", "first-test.json")) as f:
            data = v.encode_legacy(json.load(f), 1)
        self.assertEqual(cpp_array("first", "store_edit.cpp"), data)
        self.assertEqual(cpp_array("vartest", "page.cpp"), data)
        self.assertEqual(cpp_array("vartest", "extension_page.cpp"), data)
        self.assertEqual(cpp_array("saved", "extension.cpp"), data)


class Gestures(unittest.TestCase):
    def test_fades(self):
        # Build 016 fixed the fades to the volume (gesture ids 8 and 9); build 039 freed them to take any target,
        # starting on the volume. This tool went on refusing any other target until build 062 ("I thought we
        # made it possible to fade other targets, why is that no longer supported?"), so a fade on another target is
        # asserted below: the device has always allowed it since 039.
        base = {"song": 1, "entry": 1, "offset": 0, "length": 32}
        doc = {"variations": [dict(base, lanes=[{"gesture": "fade in", "amount": 255, "tracks": [1]},
                                                {"gesture": "fade out", "amount": 128, "tracks": [2], "curve": 20}])]}
        data = v.encode(doc)
        self.assertEqual(data[2], 12)
        self.assertEqual((data[13], data[15]), (8, 15))
        lanes = v.decode(data)["variations"][0]["lanes"]
        self.assertEqual([(l["gesture"], l["target"], l["curve"]) for l in lanes],
                         [("fade in", "volume", 60), ("fade out", "volume", 20), ("none", None, 0)])  # build 060: lane C empty
        self.assertEqual(lanes[0]["line"], [{"step": 0, "value": 0}, {"step": 31, "value": 128, "curve": 60}])

        self.assertEqual(lanes[1]["line"], [{"step": 0, "value": 128}, {"step": 31, "value": 64, "curve": 20}])
        # 50 %: from halfway down.
        half = v.lane_line("fade in", 128, 16, [3])
        self.assertEqual(half[0]["value"], 64)
        # Build 036: fade in arrives at as set from below; rise now leaves as set, up.
        self.assertEqual([p["value"] for p in v.lane_line("fade in", 200, 16, [3], 40)], [28, 128])
        # Build 062 ("I thought we made it possible to fade other targets, why is that no longer supported?").
        # Build 016 fixed the fades to the volume and build 039 freed them; this tool went on refusing any other
        # target until now, so where it used to raise it must now write the lane and read it back unchanged.
        other = v.encode({"variations": [dict(base, lanes=[{"gesture": "fade in", "target": "cutoff",
                                                            "amount": 255, "tracks": [1]}])]})
        got = v.decode(other)["variations"][0]["lanes"][0]
        self.assertEqual((got["gesture"], got["target"]), ("fade in", "cutoff"))

    def test_same_bytes_as_gestures_hpp(self):
        # tests/gestures.cpp checks gestures.hpp writes these bytes for the same variations.
        self.assertEqual(cpp_array("expected", "gestures.cpp"), v.encode(fixture("gestures.json")))

    def test_lines_generated_when_absent(self):
        doc = v.decode(v.encode(fixture("gestures.json")))
        build = doc["variations"][0]["lanes"][0]
        self.assertEqual((build["tracks"], len(build["line"])), ([1, 2, 3], 3))
        self.assertEqual(doc["variations"][5]["lanes"][1]["line"], [])  # none
        given = dict(fixture("gestures.json")["variations"][0])
        given["lanes"] = [dict(given["lanes"][0], line=[])]
        self.assertEqual(v.decode(v.encode({"variations": [given]}))["variations"][0]["lanes"][0]["line"], [])

    def test_build_and_break(self):
        # One line, the leading track's; the others follow it later (the stagger).
        line = v.lane_line("build", 255, 64, [1, 2, 3, 4])
        entries = [line[-1]["step"] + v.stagger(1, 255, 0, 0b1111, t, 64) for t in range(4)]
        self.assertEqual(entries, sorted(entries))
        self.assertTrue(all(e % 4 == 0 for e in entries) and entries[0] <= 8 and entries[-1] >= 52)
        self.assertTrue(line[0]["value"] == 0 and line[-1]["value"] == 128)
        brk = v.lane_line("break", 255, 64, [1, 2, 3, 4])
        exits = [brk[-1]["step"] + v.stagger(2, 255, 0, 0b1111, t, 64) for t in range(4)]
        self.assertEqual(exits, sorted(exits, reverse=True))  # the highest track leaves first
        # Every other gesture: all tracks alike.
        self.assertEqual({v.stagger(g, 255, 0, 0b1111, t, 64) for g in (0, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13) for t in range(4)}, {0})

    def test_random(self):
        line = v.lane_line("random", 200, 32, [1], seed=7)
        # Sample and hold: a hold point on the step before each beat, then the new value as a jump.
        self.assertEqual([p["step"] for p in line], [0] + [s for b in range(4, 32, 4) for s in (b - 1, b)])
        self.assertTrue(all(p.get("curve", 0) == (127 if p["step"] % 4 == 0 and p["step"] else 0) for p in line))
        self.assertTrue(all(line[k]["value"] == line[k - 1]["value"] for k in range(1, len(line), 2)))
        self.assertTrue(all(29 <= p["value"] <= 227 for p in line))
        self.assertEqual(line, v.lane_line("random", 200, 32, [1], seed=7))
        self.assertNotEqual(line, v.lane_line("random", 200, 32, [1], seed=8))
        self.assertEqual(v.lane_line("random", 0, 32, [1]), [])
        self.assertEqual(len(v.lane_line("random", 255, 128, [1])), 31)  # every 2 beats in 8 bars: within 32

    def test_random_amount_scales_the_same_pattern(self):
        # Build 042: a new amount deepens or flattens the same values (same seed), never reshuffles them.
        deep = [p["value"] - 128 for p in v.lane_line("random", 255, 32, [1], seed=5)]
        half = [p["value"] - 128 for p in v.lane_line("random", 128, 32, [1], seed=5)]
        self.assertEqual(len(deep), len(half))
        for d, h in zip(deep, half):
            self.assertTrue(abs(h) <= abs(d) + 1 and (h == 0 or d == 0 or (h > 0) == (d > 0)))
        order = lambda xs: sorted(range(len(xs)), key=lambda k: (xs[k], k))
        self.assertEqual(order(deep), order(half))

    def test_wander(self):
        line = v.lane_line("wander", 255, 32, [1], seed=3)
        self.assertEqual([p["step"] for p in line], [0, 8, 16, 24, 31])
        self.assertEqual((line[0]["value"], line[-1]["value"]), (128, 128))
        self.assertTrue(all("curve" not in p for p in line))
        self.assertEqual(v.lane_line("wander", 0, 32, [1]), [])
        self.assertEqual(len(v.lane_line("wander", 255, 128, [1])), 17)

    def test_cut_in(self):
        for amount, gap in ((0, 4), (128, 10), (255, 16)):
            line = v.lane_line("cut in", amount, 32, [1])
            self.assertEqual(line[0], {"step": 0, "value": 0})
            self.assertEqual(line[-1], {"step": gap, "value": 128, "curve": 127})
        self.assertEqual(v.natural_target(12), v.VOLUME)

    def test_gate(self):
        # Build 044: a gate over the window: a beat on, a beat off (the amount's dip), one bar stored and
        # stretched over the window's steps; the rate sets its speed in playback.
        self.assertEqual([(p["step"], p["value"]) for p in v.lane_line("gate", 255, 16, [1])], [(0, 128), (3, 128), (4, 0), (7, 0), (8, 128), (11, 128), (12, 0)])
        self.assertEqual([(p["step"], p["value"]) for p in v.lane_line("gate", 128, 32, [1])], [(0, 128), (7, 128), (8, 64), (15, 64), (16, 128), (23, 128), (24, 64)])
        self.assertEqual([p["step"] for p in v.lane_line("gate", 255, 128, [1])], [0, 31, 32, 63, 64, 95, 96])
        self.assertEqual(v.lane_line("gate", 0, 32, [1]), [])
        self.assertEqual([p["step"] for p in v.lane_line("gate", 255, 8, [1])], [0, 3, 4])
        self.assertEqual(v.natural_target(13), v.VOLUME)

    def test_hold(self):
        # Build 062 ("it might be good to have a flat line gesture"): gesture 18, by name "hold".
        self.assertEqual(v.GESTURES.index("hold"), 18)
        self.assertEqual(v.natural_target(18), v.VOLUME)
        self.assertEqual(v.default_curve("hold"), 0)
        for length in (8, 32, 256):
            self.assertEqual([(p["step"], p["value"], p.get("curve", 0)) for p in v.lane_line("hold", 255, length, [1])],
                             [(0, 0, 0), (length - 1, 0, 0)])
        self.assertEqual([p["value"] for p in v.lane_line("hold", 128, 32, [1])], [64, 64])
        self.assertEqual(v.lane_line("hold", 0, 32, [1]), [])
        lane = {"gesture": "hold", "amount": 255, "tracks": [1, 3]}
        doc = v.decode(v.encode({"variations": [dict(BUILD["variations"][0], lanes=[lane])]}))
        got = doc["variations"][0]["lanes"][0]
        self.assertEqual((got["gesture"], got["amount"], got["tracks"], got["target"], got["curve"]), ("hold", 255, [1, 3], "volume", 0))
        self.assertEqual([(p["step"], p["value"]) for p in got["line"]], [(0, 0), (BUILD["variations"][0]["length"] - 1, 0)])
        cut = dict(lane, target="cutoff")  # any target, unlike the fades before build 039
        doc = v.decode(v.encode({"variations": [dict(BUILD["variations"][0], lanes=[cut])]}))
        self.assertEqual(doc["variations"][0]["lanes"][0]["target"], "cutoff")

    def test_stutter(self):
        # Build 046: gesture 14, a note retrigger: no line, on the notes, round-trips.
        self.assertEqual(v.GESTURES.index("stutter"), 14)
        self.assertEqual(v.GESTURES.index("gate"), 13)
        # Build 047: its line is the density, whatever the amount (gestures.hpp's mirror).
        # Build 048: that line is "spring"; stutter is steady again.
        line = v.lane_line("spring", 255, 32, [1])
        self.assertEqual([(p["step"], p["value"]) for p in line], [(0, 128), (24, 255), (25, 170), (27, 225), (29, 150), (31, 128)])
        self.assertEqual(line[1]["curve"], 60)
        self.assertEqual(line, v.lane_line("spring", 0, 32, [1]))
        self.assertEqual([(p["step"], p["value"]) for p in v.lane_line("spring", 255, 8, [1])], [(0, 128), (6, 255), (7, 128)])
        self.assertEqual([(p["step"], p["value"]) for p in v.lane_line("stutter", 255, 8, [1])], [(0, 255), (7, 255)])
        self.assertEqual(v.GESTURES.index("stutter out"), 17)
        self.assertEqual(v.natural_target(17), v.NOTES)
        self.assertEqual(v.lane_line("stutter", 255, 32, []), [])
        self.assertEqual(v.natural_target(14), v.NOTES)
        lane = {"gesture": "stutter", "amount": 100, "tracks": [1, 2], "rate": 4}
        doc = v.decode(v.encode({"variations": [dict(BUILD["variations"][0], lanes=[lane])]}))
        out = doc["variations"][0]["lanes"][0]
        self.assertEqual((out["gesture"], out["amount"], out["tracks"], out["rate"], out["line"]), ("stutter", 100, [1, 2], 4, v.lane_line("stutter", 100, 32, [1, 2])))

    def test_drop_gap(self):
        for amount, gap in ((0, 4), (128, 10), (255, 16)):
            line = v.lane_line("drop", amount, 32, [1])
            self.assertEqual(32 - line[-1]["step"], gap)
            self.assertEqual(line[-1]["value"], 0)

    def test_shapes(self):
        fall = v.lane_line("fall", 255, 32, [1])
        rise = v.lane_line("rise", 255, 32, [1])
        self.assertTrue(fall[-1]["value"] == 0 and fall[-1]["curve"] < 0)
        self.assertTrue(rise[0]["value"] == 128 and rise[-1]["value"] == 255 and rise[-1]["curve"] > 0)
        swell = v.lane_line("swell", 255, 32, [1])
        dip = v.lane_line("dip", 255, 32, [1])
        self.assertEqual([p["value"] for p in swell], [128, 255, 128])
        self.assertEqual([p["value"] for p in dip], [128, 0, 128])
        self.assertEqual(v.lane_line("swell", 0, 32, [1]), [])


def v4_bytes(lanes, length=64, offset=None):
    """A version 4 store of one variation whose lanes are given as (gesture index, amount, curve, tracks
    (0-7), flags) with each track's line as build 023 made it (a jump a track at turn_step(r))."""
    offset = -length if offset is None else offset
    out = bytearray(b"VR\x04\x01") + bytes([0, 1]) + (offset & 0xffff).to_bytes(2, "little") + bytes([length, 0, 0, 255, 255])
    for g, amount, curve, tracks, flags in lanes:
        selection = sum(1 << t for t in tracks)
        order = sorted(tracks, reverse=(g == 2))
        runs = []
        for r, t in enumerate(order):
            at = v.turn_step(r, len(order), amount, length, curve)
            start, end = (0, 128) if g == 1 else (128, 0)
            pts = [(0, start, 0)] + ([(at - 1, start, 0)] if at > 1 else []) + [(at, end, 127)]
            runs.append((t, pts))
        out += bytes([g, amount, 15, selection, flags, curve & 0xff, len(runs)])
        for t, pts in sorted(runs):
            out += bytes([t, len(pts)])
            for step, value, c in pts:
                out += bytes([step, value, c & 0xff])
    return bytes(out)


class Stagger(unittest.TestCase):
    def test_reproduces_version_4_entries_and_exits(self):
        # Build 023's per-track jumps, upgraded to one line a lane: every track, at every step, asks what
        # it asked before (store.cpp holds store.hpp to the same).
        for g in (1, 2):
            for amount in (0, 90, 200, 255):
                for curve in (0, 60, -100):
                    for tracks in ([0], [0, 1], [1, 3, 4, 6], list(range(8))):
                        for length in (8, 32, 128):
                            old = v4_bytes([(g, amount, curve, tracks, 3), (g, amount, curve, tracks, 1 | 4)], length)
                            doc = v.decode(old)
                            lane = doc["variations"][0]["lanes"][0]
                            for step in range(length):
                                got = v.track_values(lane, length, step)
                                for t in tracks:
                                    order = sorted(tracks, reverse=(g == 2))
                                    at = v.turn_step(order.index(t), len(order), amount, length, curve)
                                    want = (128 if step >= at else 0) if g == 1 else (0 if step >= at else 128)
                                    self.assertEqual(got[t + 1], want << 8, (g, amount, curve, tracks, length, step, t))

    def test_edited_takes_the_leading_tracks_line(self):
        # An edited break: its line the highest track's (the first to leave), the others following later.
        old = v4_bytes([(2, 200, 0, [1, 5], 4), (0xff, 0, 0, [], 2)], 32)
        lane = v.decode(old)["variations"][0]["lanes"][0]
        self.assertTrue(lane["edited"])
        at = v.turn_step(0, 2, 200, 32, 0)
        self.assertEqual(lane["line"][-1]["step"], at)

    def test_custom_keeps_the_tracks_with_points(self):
        old = bytearray(v4_bytes([(0xff, 0, 0, [], 2), (0xff, 0, 0, [], 2)], 32))
        # lane a: custom on tracks 1-3 selected, points on tracks 3 and 5 (0-based 2 and 4).
        lane = bytes([0, 0, 15, 0b111, 0, 0, 2, 2, 1, 4, 64, 0, 4, 1, 9, 200, 0])
        old = bytes(old[:13]) + lane + bytes(old[20:])
        doc = v.decode(old)
        a = doc["variations"][0]["lanes"][0]
        self.assertEqual((a["tracks"], a["line"]), ([3, 5], [{"step": 4, "value": 64}]))


class LongWindows(unittest.TestCase):
    """Version 9 (build 049): windows to 1024 steps, a line of its own length, reach to 31."""

    def window(self, length, **extra):
        lane = {"gesture": "rise", "amount": 255, "target": "cutoff", "tracks": [1]}
        return {"variations": [dict(BUILD["variations"][0], **dict(dict(length=length, lanes=[lane]), **extra))]}

    def test_lengths(self):
        for length in (512, 1024):
            data = v.encode(self.window(length))
            self.assertEqual(data[2], 12)
            self.assertEqual(v.decode(data)["variations"][0]["length"], length)
            self.assertEqual(v.encode(v.decode(data)), data)

    def test_line_is_at_most_256_steps(self):
        self.assertEqual([v.line_length(n) for n in (8, 256, 257, 1024)], [8, 256, 256, 256])
        line = v.decode(v.encode(self.window(1024)))["variations"][0]["lanes"][0]["line"]
        self.assertEqual([p["step"] for p in line], [0, 255])
        with self.assertRaises(v.FormatError):
            v.encode(self.window(1024, lanes=[{"gesture": "custom", "target": "cutoff", "tracks": [1],
                                               "line": [{"step": 256, "value": 0}]}]))
        v.encode(self.window(1024, lanes=[{"gesture": "custom", "target": "cutoff", "tracks": [1],
                                           "line": [{"step": 255, "value": 0}]}]))

    def test_stretch_and_gaps(self):
        # A rise over 1024 steps is halfway at step 512, and at its top by the window's end.
        lane = {"gesture": "rise", "amount": 255, "curve": 0, "tracks": [1],
                "line": v.lane_line("rise", 255, 1024, [1], 0)}
        mid, end = (v.track_values(lane, 1024, at)[1] for at in (512, 1023))
        self.assertTrue(abs(mid - (128 + 127 // 2) * 256) < 400 and end >= 254 * 256)
        # The drop's gap stays real time: 4..16 window steps, 1..4 line steps of a 4-step line.
        self.assertEqual([v.drop_gap(a, 1024) for a in (0, 255)], [1, 4])
        self.assertEqual([v.drop_gap(a, 256) for a in (0, 255)], [4, 16])
        # The gate stores one bar over the 256-step line, a beat on (64 line steps) and a beat off.
        self.assertEqual([p["step"] for p in v.lane_line("gate", 255, 1024, [1])], [0, 63, 64, 127, 128, 191, 192])

    def test_version_8_loads(self):
        data = bytearray(v.encode(self.window(256)))
        data = as_old(data, 8)
        self.assertEqual(v.decode(bytes(data))["variations"][0]["length"], 256)
        self.assertEqual(v.encode(v.decode(bytes(data))), bytes(as_current(data)))
        # Version 8 keeps its ranges: a 512-step window or a 16-join reach is no version 8.
        long = bytearray(v.encode(self.window(512)))
        long = as_old(long, 8)
        with self.assertRaises(v.FormatError):
            v.decode(bytes(long))

    def test_reach_to_31(self):
        reach = [{"scene": 4, "start": 10 * (s + 1)} for s in range(31)]
        doc = self.window(1024, reach=reach)
        self.assertEqual(v.decode(v.encode(doc))["variations"][0]["reach"], reach)
        # Build 063 (version 12): a start above 4095 is allowed now, to 12286 (a window of 4096 from offset 8191).
        far = self.window(4096, offset=8191, reach=[{"scene": 4, "start": 12286}])
        self.assertEqual(v.decode(v.encode(far))["variations"][0]["reach"], [{"scene": 4, "start": 12286}])
        with self.assertRaises(v.FormatError):
            v.encode(self.window(4096, offset=8191, reach=[{"scene": 4, "start": 12287}]))


class SongSpanning(unittest.TestCase):
    """Version 12 (build 063: "I'd like a song spanning variation to be an option"): windows to 4096 steps (256 bars),
    95 joins each way, offsets to 8191; version 11 and below read within their own ranges."""

    def doc(self, **kw):
        v0 = dict(BUILD["variations"][0], **kw)
        v0["lanes"] = [dict(l) for l in v0["lanes"]]
        return {"variations": [v0]}

    def test_lengths(self):
        for length in (2048, 4096):
            data = v.encode(self.doc(length=length))
            self.assertEqual(v.decode(data)["variations"][0]["length"], length)
            self.assertEqual(v.encode(v.decode(data)), data)
        with self.assertRaises(v.FormatError):
            v.encode(self.doc(length=4097))

    def test_both_lists_at_95(self):
        reach = [{"scene": 1 + s % 98, "start": 32 * (s + 1)} for s in range(95)]
        back = [{"scene": None if s % 7 == 0 else 1 + s % 98, "start": 32 * (s + 1)} for s in range(95)]
        doc = self.doc(length=4096, offset=-3041, reach=reach, back=back)
        data = v.encode(doc)
        self.assertEqual(data[4 + 9], 95)
        self.assertEqual(data[4 + 9 + 1 + 3 * 95], 95)
        self.assertEqual(v.decode(data)["variations"][0]["reach"], reach)
        self.assertEqual(v.decode(data)["variations"][0]["back"], back)
        self.assertEqual(v.encode(v.decode(data)), data)
        self.assertEqual(len(data), len(v.encode(self.doc(length=4096))) + 2 * (1 + 3 * 95))  # 286 bytes a list

    def test_version_11_keeps_its_ranges(self):
        # Version 11 bytes are version 12's within the smaller ranges: they decode, and encode as version 12.
        data = v.encode(self.doc(length=1024, offset=2047, reach=[{"scene": 4, "start": 2100}]))
        old = as_old(data, 11)
        self.assertEqual(old[2], 11)
        self.assertEqual(v.encode(v.decode(bytes(old))), data)
        # A version 11 window of 2048 steps, or an offset of 8191, is no version 11.
        for broken in (v.encode(self.doc(length=2048)), v.encode(self.doc(offset=8191, length=64))):
            with self.assertRaises(v.FormatError):
                v.decode(bytes(as_old(broken, 11)))
        # 32 reach records (and a reach start above 4095) are no version 11 either.
        wide = v.encode(self.doc(length=1024, reach=[{"scene": 4, "start": s + 1} for s in range(32)]))
        with self.assertRaises(v.FormatError):
            v.decode(bytes(as_old(wide, 11)))
        v.decode(wide)


class BackReach(unittest.TestCase):
    """Version 10 (build 058): a backward reach list, after the forward one."""

    def doc(self, **kw):
        v0 = dict(BUILD["variations"][0], length=300, offset=-200, **kw)
        v0["lanes"] = [dict(l) for l in v0["lanes"]]
        return {"variations": [v0]}

    def test_round_trip(self):
        reach = [{"scene": 4, "start": 64}, {"scene": None, "start": 90}]
        back = [{"scene": 4, "start": 64}, {"scene": None, "start": 128}, {"scene": 2, "start": 192}]
        data = v.encode(self.doc(reach=reach, back=back))
        self.assertEqual(data[2], 12)
        self.assertEqual(data[4 + 6] & 12, 12)  # both flags
        # The forward list, then the backward one: count, then {scene, u16 start}.
        self.assertEqual(data[4 + 9:4 + 9 + 7], bytes([2, 3, 64, 0, 255, 90, 0]))
        self.assertEqual(data[4 + 16:4 + 16 + 10], bytes([3, 3, 64, 0, 255, 128, 0, 1, 192, 0]))
        out = v.decode(data)["variations"][0]
        self.assertEqual((out["reach"], out["back"]), (reach, back))
        self.assertEqual(v.encode(v.decode(data)), data)
        only = v.encode(self.doc(back=back))
        self.assertEqual(only[4 + 6] & 12, 8)
        self.assertEqual(only[4 + 9:4 + 9 + 4], bytes([3, 3, 64, 0]))
        self.assertEqual(v.decode(only)["variations"][0]["back"], back)
        plain = v.encode(self.doc())
        self.assertEqual(plain[4 + 6] & 8, 0)
        self.assertNotIn("back", v.decode(plain)["variations"][0])

    def test_refused(self):
        for bad in ([{"scene": 4, "start": 0}], [{"scene": 4, "start": 64}, {"scene": 5, "start": 64}],
                    [{"scene": 100, "start": 64}], [{"scene": 4, "start": 12287}],
                    [{"scene": 4, "start": 201}],  # build 064: the window (offset -200) does not reach an entry 201 back
                    [{"scene": 4, "start": s + 1} for s in range(96)]):
            with self.assertRaises(v.FormatError, msg=bad):
                v.encode(self.doc(back=bad))
        v.encode(self.doc(back=[{"scene": 4, "start": 199}]))
        v.encode(self.doc(back=[{"scene": 4, "start": 200}]))  # build 064: starting exactly on that entry's start reaches it
        data = bytearray(v.encode(self.doc(back=[{"scene": 4, "start": 64}])))
        for at, value in ((4 + 9, 0), (4 + 9, 32), (4 + 10, 99), (4 + 11, 0)):
            broken = bytearray(data)
            broken[at] = value
            if at == 4 + 11:
                broken[at + 1] = 0
            with self.assertRaises(v.FormatError, msg=(at, value)):
                v.decode(bytes(broken))

    def test_version_9_loads(self):
        # Version 9 bytes have no backward list: the flag, meaningless then, is ignored, and they read as version 10.
        data = bytearray(v.encode(self.doc(reach=[{"scene": 4, "start": 64}])))
        data = as_old(data, 9)
        out = v.decode(bytes(data))["variations"][0]
        self.assertNotIn("back", out)
        self.assertEqual(out["reach"], [{"scene": 4, "start": 64}])
        self.assertEqual(v.encode(v.decode(bytes(data))), bytes(as_current(data)))
        data[4 + 6] |= 8
        self.assertNotIn("back", v.decode(bytes(data))["variations"][0])

    def test_back_to_31(self):
        # Build 063 (version 12): up to 95 records now; 31 still round trips, and a 32nd is no longer refused.
        back = [{"scene": 4, "start": 10 * (s + 1)} for s in range(31)]
        doc = self.doc(back=back)
        doc["variations"][0].update(offset=-400)
        self.assertEqual(v.decode(v.encode(doc))["variations"][0]["back"], back)
        doc["variations"][0]["back"] = back + [{"scene": 4, "start": 320}]
        self.assertEqual(len(v.decode(v.encode(doc))["variations"][0]["back"]), 32)

    def test_back_to_95(self):
        # Build 063 (version 12): 95 joins back; a 96th is refused.
        back = [{"scene": 4, "start": 10 * (s + 1)} for s in range(95)]
        doc = self.doc(back=back)
        doc["variations"][0].update(offset=-1000)
        self.assertEqual(v.decode(v.encode(doc))["variations"][0]["back"], back)
        doc["variations"][0]["back"] = back + [{"scene": 4, "start": 960}]
        with self.assertRaises(v.FormatError):
            v.encode(doc)


class StoreEdits(unittest.TestCase):
    def test_edited_bytes_decode(self):
        # tests/store_edit.cpp edits vartest's store into these bytes.
        data = cpp_array("expected", "store_edit.cpp")
        self.assertEqual(v.encode(fixture("edits.json")), data)
        self.assertEqual(v.encode(v.decode(data)), data)

    def test_fixtures_are_current(self):
        data = v.encode(BUILD)
        with open(os.path.join(HERE, "fixtures", "build.bin"), "rb") as f:
            self.assertEqual(f.read(), data)
        self.assertEqual(cpp_array("data", "store.cpp"), data)
        self.assertEqual(cpp_array("two", "extension.cpp"), v.encode(fixture("two-lanes.json")))
        self.assertEqual(cpp_array("onjoin", "extension.cpp"), v.encode(fixture("build-on-join.json")))
        # Build 060: the same fixtures as version 10 saved them (two lanes a variation), which load with lane C empty.
        self.assertEqual(cpp_array("data10", "store.cpp"), bytes(as_old(data, 10)))
        self.assertEqual(cpp_array("two10", "extension.cpp"), bytes(as_old(v.encode(fixture("two-lanes.json")), 10)))
        self.assertEqual(as_current(cpp_array("data10", "store.cpp")), data)
        # Build 063: the same as version 11 saved it (the smaller ranges; only the number differs).
        self.assertEqual(cpp_array("data11", "store.cpp"), bytes(as_old(data, 11)))

    def test_legacy_four_lines(self):
        # Build 060: store.cpp's version 1 and 2 data with every line (four, then three) and the tool's upgrade of it:
        # lines in pairs, lane A then B, lane C empty, a third and fourth line a variation of their own.
        legacy = fixture("legacy-four.json")
        self.assertEqual(cpp_array("legacy4v2", "store.cpp"), v.encode_legacy(legacy, 2))
        self.assertEqual(cpp_array("legacy4v1", "store.cpp"), v.encode_legacy(legacy, 1))
        self.assertEqual(cpp_array("upgraded4", "store.cpp"), v.upgrade(v.encode_legacy(legacy, 2)))
        doc = v.decode(v.encode_legacy(legacy, 1))
        self.assertEqual([[l["target"] for l in x["lanes"]] for x in doc["variations"]],
                         [["volume", "notes", None], ["fx i send", "cutoff", None],
                          ["volume", "fx i send", None], ["cutoff", None, None]])
        self.assertEqual(v.encode(doc), v.upgrade(v.encode_legacy(legacy, 1)))


class Artwork(unittest.TestCase):
    def test_current(self):
        result = subprocess.run([sys.executable, os.path.join(ROOT, "tools", "page_svg.py"), "--check"])
        self.assertEqual(result.returncode, 0, "variations_page.svg: run tools/page_svg.py")

    def test_bound_as_declared(self):
        with open(os.path.join(ROOT, "extension.json")) as f:
            display = json.load(f)["page"]["display"]
        with open(os.path.join(ROOT, "variations_page.svg")) as f:
            svg = f.read()
        for kind, count in (("node", display["nodes"]), ("text", display["texts"]), ("palette", display["palette"])):
            ids = re.findall(r'id="variations_' + kind + r'_(\d+)"', svg)
            self.assertEqual(sorted(map(int, ids)), list(range(count)), kind)
        for path in range(display["paths"]):
            shape = re.search(r'id="variations_node_%d" d="([^"]*)"' % path, svg).group(1)
            self.assertEqual(len(re.findall(r"[ML]", shape)), display["points"])

    # Build 022: stock's text sizes only (its artwork, private/stock-art: 20, 10
    # and 12 px most, then 15, 17, 30, 40); 021's 9, 11, 13 and 26 px looked aliased.
    STOCK_SIZES = {10, 12, 15, 17, 20, 30, 40}

    def test_stock_text_sizes(self):
        with open(os.path.join(ROOT, "variations_page.svg")) as f:
            svg = f.read()
        texts = re.findall(r'<text id="variations_text_(\d+)" transform="([^"]*)"[^>]*font-size="([0-9.]+)px"', svg)
        self.assertEqual(len(texts), 16)
        for slot, transform, size in texts:
            self.assertIn(float(size), self.STOCK_SIZES, "text %s: %spx" % (slot, size))
            self.assertTrue(transform.startswith("matrix(1 0 0 1 "), "text %s unscaled" % slot)

    def test_page_hpp_sizes_match_the_svg(self):
        # page.hpp's sizes (texts placed and fitted by them) are the svg's slots'.
        with open(os.path.join(ROOT, "page.hpp")) as f:
            hpp = f.read()
        sizes = {m.group(1): int(m.group(2)) for m in re.finditer(r"\b(\w+_size) = (\d+)", hpp)}
        with open(os.path.join(ROOT, "variations_page.svg")) as f:
            svg = f.read()
        slot = {int(i): float(px) for i, px in re.findall(r'<text id="variations_text_(\d+)"[^>]*font-size="([0-9.]+)px"', svg)}
        # page.hpp screen::Text order (build 052: the strip's slots, names 10 px and values 15; build 054: two 20 px
        # M-key marks, the second out of the window's end bar number; build 056: a third, over M2, out of the header's
        # track list).
        order = ["t_title", "t_mark_m2", "t_join", "t_label_a", "t_label_b", "t_number_a", "t_mark_side", "t_mark",
                 "t_name_1", "t_name_2", "t_name_3", "t_name_4", "t_value_1", "t_value_2", "t_value_3", "t_value_4"]
        expect = {"t_title": "header_size", "t_mark_m2": "mark_size", "t_join": "join_size", "t_label_a": "label_size",
                  "t_label_b": "label_size", "t_number_a": "label_size", "t_mark_side": "mark_size", "t_mark": "mark_size",
                  "t_name_1": "names_size", "t_name_2": "names_size", "t_name_3": "names_size", "t_name_4": "names_size",
                  "t_value_1": "value_size", "t_value_2": "value_size", "t_value_3": "value_size", "t_value_4": "value_size"}
        for i, name in enumerate(order):
            self.assertEqual(slot[i], sizes[expect[name]], name)
        for name, size in sizes.items():
            self.assertIn(size, self.STOCK_SIZES, name)


if __name__ == "__main__":
    unittest.main()
