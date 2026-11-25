# Variations

A few bars of change over a song, for the OP-XY on OS 1.2.1.

Pin a variation to a join between two song entries and it plays a shaped move
through the bars around it: tracks fade, break and build, or any of their
sounds' controls rises, falls, swells or dips. The song itself is untouched —
nothing is written into a pattern, and the variation plays only where it is
pinned.

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

Press **M4** on the song page to open the variation page over the join under
the cursor: a timeline of the song with the window on it, the lanes' shapes,
and the controls to move, shape and retarget them. Joins that hold a variation
are marked on the song page itself, so you can see where they are without
opening anything.

## Limits

The ones you meet first, in the order you meet them:

| | |
| --- | --- |
| Variations in a project | **64** |
| Saved state | **8192 bytes**; an ordinary variation costs 30 |
| Window length | 4096 steps (256 bars) |
| Joins a window may reach, each way | 95 |
| Lanes a variation | 3 |
| Points on a drawn line | 32 |
| Words driven in one audio pass | **32**, the SDK's cap |

The last one is worth knowing: one variation at three lanes across eight tracks
asks for 24 of the 32 words a pass, so two overlapping variations can ask for
more than the pass can send, and the excess is dropped without a sign. The
others you will not reach by accident.

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
