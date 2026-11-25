# Variations

Adds variations to song mode to allow three lanes of assignable LFO like transitions
over time between scenes, but independent of the scene itself for the OP-XY on OS 1.2.1.

Pin a variation between two song entries and it plays a shaped movement
through the bars around it: tracks fade, break and build, rises, falls,
swells or dips. The song itself is untouched — nothing is written into a
pattern, and the variation plays only where it is pinned. It is freely
movable, and can potentially run the length of a song.

Built with the [Open-XY SDK](https://github.com/etcetc000/open-xy) and reaching
the [builder](https://open-xy.com) through its catalog. It uses only the public
`<opxy/extension.hpp>` events and actions; it patches no firmware and names no
stock addresses.

## What it does

A variation has a **window** — a position and a length in bars, anywhere around
the join it is pinned to — and up to **three lanes**. Each lane moves one
target on its own tracks:

| | |
| --- | --- |
| **Targets** | A track's volume, cutoff, resonance, LFO speed and amount, envelope stages, the sends, or any control of its engine, chosen by name so the lane follows that control wherever a track's engine keeps it |
| **Gestures** | fade in, fade out, rise, fall, swell, dip, build, break, drop, cut in, gate and stutter (a note retrigger), each with an amount and a curve |
| **Drawn lines** | Any lane can be drawn by hand instead, up to 32 points, with a curve between each pair |
| **Tracks** | Any of the eight instrument tracks; build and break stagger theirs, so they arrive one after another |

Press **M4** on the song page to open the variation page between the scenes under
the cursor: a timeline of the song with the window on it, the lanes' shapes,
and the controls to move, shape and retarget them. Joins that hold a variation
are marked on the song page itself, so you can see where they are without
opening anything.

## Limits

A **control** is one knob on one track: track 1's volume, track 3's cutoff. It
is the unit the OP-XY moves things in, and the one the limits below count. A
lane is one knob plus the tracks it moves, so a lane across all eight tracks is
eight controls, and a variation using all three lanes across all eight tracks
is 24.

| | |
| --- | --- |
| Variations in a project | **64** |
| Saved state | **8192 bytes**; an ordinary variation costs 30 |
| Window length | 256 bars |
| Joins a window may reach, each way | 95 |
| Lanes a variation | 3 |
| Points on a drawn line | 32 |
| Controls moving at once | **32**, the OP-XY's own ceiling |

Every limit but the last is one you would have to go looking for. The last one
you can meet by accident, so it is worth knowing how.

One variation asks for 24 controls at the very most, so on its own it always
fits. But two variations over the same bars ask for both their sets at once —
up to 48 between them, where only 32 can move. The sixteen over the ceiling do
not move, and today nothing tells you: a fade you set up simply does not fade.

## Checks

Clone the SDK next to this repository, then from here:

```sh
python ../open-xy/sdk.py test --extension .
```

That runs the host tests in `tests/` (the extension end to end on the SDK's
scene-clock and track-value doubles, the page's drawing and input, the store's
format and its upgrades) and the `variations.py` tool's own tests.

The page fuzzers in `tools/fuzz` drive random input at the page and check that
every frame it produces is drawable; run them after any change to the page's
drawing or its bounds. See [tools/fuzz/README.md](tools/fuzz/README.md).

## The saved bytes

A project's variations live in its companion, in the `open-xy.extensions`
section. `tools/variations.py` reads and writes them as JSON, which is the way
to put a prepared set of variations on a project:

```sh
python tools/variations.py encode song.json song.bin
python ../open-xy/sdk.py project state song.xy --extension variations --set song.bin
```

The format is documented at the top of that file, including every version it
still reads; older projects load unchanged and are renumbered as they load.
`examples/first-test.json` is a small worked example.

## Licence

MIT — see [LICENSE](LICENSE).
