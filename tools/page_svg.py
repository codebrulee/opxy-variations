#!/usr/bin/env python3
"""Writes the variation page's artwork, variations_page.svg (page.hpp's layout).

    python tools/page_svg.py            # writes ../variations_page.svg
    python tools/page_svg.py --check    # exit 1 if the file is not current

One 480 x 220 page, greyscale, as few elements as the page needs: the bound
nodes are the two lanes' motions (64-point outlines: the other lane's dim,
behind, then the lane shown), the variation page's join (build 049: the
plot's height, 2 px wide), a black strip over the background's ruler line
(the shape page's, which the timeline covers), 16 ruler marks (the shape
page's bar lines), 28 plot-high marks (were the window's tiles: build 049's
other section boundaries) and the playhead (the shape page's ruler mark);
the floor under the motions, unused since build 049; the points page's cursor and 32 points (build 024: no
dim lines of the lane's other tracks, which follow its one line); build 021's quiet hints (three hollow handles
on the shape, encoder 2's dot (build 022; 021's encoder 4), the lane letters
a and b drawn as a ring and a stem each) and build 022's encoder 1 dot by the
letters; 16 text slots (the encoder names among them: the Shift layer changes
them; the lane's target; the header's title and join), shared by both pages,
at stock's text sizes only (build 022: 10, 12 and 30 px here; build 023 the
message 12 px, build 024 10 again); build 023's shape page grid (32 full-height lines) and dashed
as set line (9 pieces), drawn first; 9 palette entries (build 023: the grid's
bar grey after the rings). What never moves (the background,
the ruler) is drawn once. Elements are drawn in file order, so the grid, the
join and the marks come before the outlines (build 049: the curves cross over
them) and the points after them; the IDs keep page.hpp's node numbers.
Build 049's text slot 2 ("gesture") is the variation page's bar numbers, 10 px
mid grey (it was the 30 px gesture name, empty since build 029).
Build 052 (2026-11-24): both pages adopt stock's engine-page language: a control strip of eight 60 x 20
greyscale ramp cells across the top (nodes 163-170; names 10 px, values 15 px in the text slots), the M-key marks along
the bottom (the lane letters at M1, two chevrons each for M2 and M3: nodes 171-174; "delete" over M4 under Shift, a
text slot) and the page's content pushed down between them (the ruler line at y 184, was 168; the join runs
12 px below the plot too). 16 palette entries (RAMP last). The text slots are renamed and rebuilt: see TEXTS.
The numbers here are page.hpp's `screen` namespace; keep the two together.
"""
import os
import sys

ID = "variations"
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "variations_page.svg")
PALETTE = ["#000000", "#323232", "#646464", "#969696", "#CDCDCD", "#F0F0F0"]
BLACK, DIM, MID, TEXT, BRIGHT, WHITE = PALETTE
# Build 021's ring entries (page.hpp s_ring_white, s_ring_mid): a black fill
# inside a 1 px stroke, for the lane letters' bowls.
RINGS = [(BLACK, WHITE, 1), (BLACK, MID, 1)]
# Build 023 (page.hpp s_grid, after the rings): the shape page's bar lines.
GRID = "#555555"
# Build 049: the variation page's window band (page.hpp s_band), darker than every line over it.
BAND = "#1A1A1A"
BAND_COUNT, BAND_WIDTH = 28, 16
# Build 050 (2026-11-24, option C1 of mockups/timeline-lines.html): the variation page's timeline greys,
# after BAND (page.hpp s_beat, s_bar, s_marker, s_join, s_other): beats fainter, bars a step up, scene markers
# (dashed) and the join (solid, 2 px) brighter, the other lane's curve under the markers.
TIMELINE = ["#262626", "#3C3C3C", "#6A6A6A", "#B4B4B4", "#5A5A5A"]  # build 051: markers subtler, #8C8C8C was
# Build 052 (2026-11-24): the control strip's one grey the palette lacked (page.hpp s_ramp, the last entry, 16th):
# stock's engine-page ramp is #000000 #323232 #555555 #646464 #7D7D7D #969696 #B4B4B4 #CDCDCD, eight 60 x 20 cells;
# the rest are BLACK, DIM, GRID, MID, TEXT, TIMELINE[3] and BRIGHT above.
RAMP = "#7D7D7D"
CELL_COUNT, CELL_W, CELL_H = 8, 60, 20
N_CELLS = 163
POLE = 12                            # the join and scene markers rise this far above the plot, to the scene names
GRID_COUNT = 32                      # the shape page's grid lines, full height
PLOT_HALF = 46                       # the plot's half height (page.hpp plot_y0..plot_y1; build 025: 56..148; build 052: 72..164)
CURSOR_HALF = 50                     # the cursor, 4 px past the plot (page.hpp cursor_half)
# The as set line's dashes: AS_SET_COUNT pieces of DASHES dashes, each DASH px on
# and GAP off, the gaps a faint CONNECT px sliver so a piece is one subpath
# (the device draws one subpath of at most 64 points a path).
DASH, GAP, DASHES, AS_SET_COUNT, CONNECT = 3, 3, 8, 9, 0.2
HANDLE_COUNT = 3
HANDLE_R, HANDLE_STROKE = 2.2, 1.2   # the shape's handles: hollow dots
# Build 022's tap hints, in the guidebook's knob colours (encoders 1-4: dark
# grey, mid grey, light grey, white), each in a lighter outline to show on black.
ENC2 = (2.5, MID, BRIGHT, 1)         # encoder 2's dot: r, fill, stroke, stroke width
ENC1 = (2, DIM, MID, 0.8)            # encoder 1's dot, by the lane letters: quieter
# Stock's text sizes (its artwork, private/stock-art): every slot uses one.
STOCK_SIZES = (10, 12, 15, 17, 20, 30, 40)
RING_R = 2.6                         # a lane letter's bowl (12 px text's x-height, 6.2 px across its stroke)
STEM_W, STEM_A, STEM_B = 0.6, 6.2, 9.0  # its stem: a's to the x-height, b's to the ascender
# Build 060 (stage B): a third lane. Its letter c is one filled glyph, a ring open on the right (a ring and a stem
# cannot make it): the same stroke weight as the other bowls (RING_R +- 0.5, a 1 px stroke), the gap RING_GAP degrees
# either side of the right. Authored last (node 171, after the strip's cells) so no earlier node moves.
N_LETTER_C = 171
RING_GAP, ARC_STEP = 50, 15
# Build 060: the lanes not being edited are sketched from nodes (page.hpp n_sketch): SKETCH_LANES of them, each
# SKETCH_PIECES thin horizontal ribbons SKETCH_LENGTH px long and SKETCH_THICK px thick, placed end to end with
# draw::segment (scale 1/16..2: a piece reaches 3 to 96 px, scaled with its thickness). 31 pieces join motion()'s 32
# samples. Their ids follow the glyph's; in the file they come before the outlines, so they draw where the other lane's
# outline did (under the markers' end, behind the lane shown).
N_SKETCH = N_LETTER_C + 1
SKETCH_LANES, SKETCH_PIECES = 2, 31
SKETCH_LENGTH, SKETCH_THICK = 48, 2
FONT = "UniversNextPro-Light"
POINTS = 64
BAR_COUNT, TILE_COUNT = 16, 28
POINT_COUNT = 32
# Text slots (page.hpp screen::Text): colour, size, authored position.
# Build 022: stock's sizes only; the header is the page's title (bright, centred),
# the lane slot the join's scenes top left (the lane letters are nodes).
# Build 052 (2026-11-24): the table is rebuilt for the control strip (eight 60 x 20 cells across the top, as
# stock's engine pages; names 10 px, values 15 px, left aligned at x 5, 125, 245, 365 and 65, 185, 305, 425; the two
# right columns on the light cells in black ink) and the M-key mark over M4. A slot carries one size and one colour,
# and scaling text is not allowed, so all 16 are spoken for. Index order is page.hpp's screen::Text.
TEXTS = [
    ("title", BRIGHT, 12, 200, 38),       # the page's title, centred (was "header", y 22)
    ("mark_m2", TEXT, 20, 175, 215),     # build 056 ("where has mute gone?"): the third M-key mark, "mute" over M2 on both pages. It was
                                          # the header's track list (12 px, BRIGHT); the track keys' LEDs carry the tracks now.
    ("join", TEXT, 12, 16, 38),           # the join's scenes, variation page only (was "lane")
    ("label_a", TEXT, 10, 16, 74),        # a scene name along the timeline; the shape page's "bar 1" (was "before")
    ("label_b", TEXT, 10, 420, 74),       # the second scene name; the shape page's last bar (was "after")
    ("number_a", TEXT, 10, 16, 187),      # the window's bar range (build 056, "5-8"; was its start bar); the shape page's centred target and its message (was "gesture")
    ("mark_side", TEXT, 20, 29, 215),   # build 054: the second M-key mark ("edit" or "back" over M3; build 056: "mute" moved to M2).
                                          # It was the window's end bar number: a 20 px slot had to come from somewhere,
                                          # and all 16 are spoken for (build 056: the end bar is back in number_a's range).
    ("mark", TEXT, 20, 406, 215),       # the M-key mark: "delete" over M4 under Shift (stock's M4 anchor 406.2)
    ("name_1", BRIGHT, 10, 5, 10),
    ("name_2", BRIGHT, 10, 125, 10),
    ("name_3", BLACK, 10, 245, 10),
    ("name_4", BLACK, 10, 365, 10),
    ("value_1", BRIGHT, 15, 65, 16.5),
    ("value_2", BRIGHT, 15, 185, 16.5),
    ("value_3", BLACK, 15, 305, 16.5),
    ("value_4", BLACK, 15, 425, 16.5),
]
assert all(size in STOCK_SIZES for _, _, size, _, _ in TEXTS)


def num(v):
    s = f"{v:.4f}".rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


def rect(x0, y0, x1, y1):
    return f"M{num(x0)},{num(y0)} L{num(x1)},{num(y0)} L{num(x1)},{num(y1)} L{num(x0)},{num(y1)} Z"


def letter_c():
    # An annulus open on the right: the outer arc round the left from RING_GAP to 360 - RING_GAP degrees, the inner back.
    import math
    outer, inner = RING_R + 0.5, RING_R - 0.5
    angles = list(range(RING_GAP, 360 - RING_GAP + 1, ARC_STEP))
    if angles[-1] != 360 - RING_GAP:
        angles.append(360 - RING_GAP)
    pts = [(outer * math.cos(math.radians(a)), outer * math.sin(math.radians(a))) for a in angles]
    pts += [(inner * math.cos(math.radians(a)), inner * math.sin(math.radians(a))) for a in reversed(angles)]
    return "M" + " L".join(f"{num(x)},{num(y)}" for x, y in pts) + " Z"


def ribbon():
    # The motion's outline as page.hpp first draws it flat: 32 points along the
    # top, 32 back 0.75 below (1.5 px at the node's scale 2), all distinct.
    half = POINTS // 2
    top = [(-60 + 120 * i / (half - 1), 0) for i in range(half)]
    bottom = [(x, 0.75) for x, _ in reversed(top)]
    pts = top + bottom
    return "M" + " L".join(f"{num(x)},{num(y)}" for x, y in pts) + " Z"


def artwork():
    out = ['<svg xmlns="http://www.w3.org/2000/svg" width="480" height="220" viewBox="0 0 480 220">']
    add = out.append
    add(f'<path d="{rect(0, 0, 480, 220)}" fill="{BLACK}"/>')
    # Build 054: no ruler line. The shape page's playhead is the variation page's now (a hairline the plot's
    # height), so neither page draws a ruler, its bar ticks or the black strip that used to cover the line here.
    # Build 049: the window band's strips (page.hpp n_band), first so behind everything placed.
    for i in range(BAND_COUNT):
        add(f'<path id="{ID}_node_{134 + i}" d="{rect(-BAND_WIDTH / 2, -PLOT_HALF, BAND_WIDTH / 2, PLOT_HALF)}" fill="{BAND}"/>')
    # page.hpp's node numbers: 0-3 the outlines; 4 the join; 5-20
    # bars; 21-48 marks; 49 the playhead; 50 the floor; 51 the cursor; 52-83
    # the points (build 024: 023's 84-139, the dim pieces, gone).
    n_floor, n_cursor = 50, 51
    n_points = n_cursor + 1
    n_handles = n_points + POINT_COUNT

    def put(i, d, fill):
        add(f'<path id="{ID}_node_{i}" d="{d}" fill="{fill}"/>')

    # Build 023, first so behind everything: the shape page's grid (after
    # encoder 1's dot: build 024 nodes 93-124) and its dashed as set line (125-133).
    n_grid = n_handles + HANDLE_COUNT + 1 + 4 + 1
    for i in range(GRID_COUNT):
        put(n_grid + i, rect(-0.5, -PLOT_HALF, 0.5, PLOT_HALF), DIM)
    top = []
    for k in range(DASHES):
        x = k * (DASH + GAP)
        top += [(x, -0.5), (x + DASH, -0.5)]
        if k + 1 < DASHES:
            top += [(x + DASH + 0.25, -CONNECT / 2), (x + DASH + GAP - 0.25, -CONNECT / 2)]
    piece = top + [(x, -y) for x, y in reversed(top)]
    assert len(piece) <= 64
    d = "M" + " L".join(f"{num(x)},{num(y)}" for x, y in piece) + " Z"
    for i in range(AS_SET_COUNT):  # from its left end, placed along the plot
        put(n_grid + GRID_COUNT + i, d, DIM)
    put(n_floor, rect(-120, -0.3, 120, 0.3), DIM)  # build 049: unused (was the variation page's floor)
    # Build 049 (2026-11-23): the variation page's timeline lines, drawn before the outlines so the curves
    # cross over them (the nodes keep their numbers; file order is drawing order).
    # Build 050: paths 2 and 3 took 049's nodes 2 (the join, now 4) and 3 (the strip, now the last node, 162).
    put(162, rect(-224, -1, 224, 1), BLACK)  # a strip over the static ruler line below, placed in black on the variation page
    put(4, rect(-1, -PLOT_HALF - POLE, 1, PLOT_HALF + POLE), TEXT)  # the join: the plot's height, a pole above it and (build 052) another below, down to the ruler, 2 px wide
    for i in range(BAR_COUNT):
        # The shape page's ruler marks. Build 052: they rise from the ruler line (-3 to 0, so y 178 to 184 at the bar
        # lines' scale 2, meeting the line at 183.7-184.3) instead of straddling it: at the bar labels' baseline 192
        # their ascenders reach 184.8, and the old marks' feet (ruler_y + 6 = 190) crossed the first label's glyphs.
        put(5 + i, rect(-0.4, -3, 0.4, 0), DIM)
    for i in range(TILE_COUNT):
        put(5 + BAR_COUNT + i, rect(-0.5, -PLOT_HALF - POLE, 0.5, PLOT_HALF), MID)  # the other section boundaries: the plot's height and a pole (build 051: solid: the dashes were lost)
    # Build 060: the sketch pieces (the two lanes not edited), before the outlines; MID like the outlines below.
    for i in range(SKETCH_LANES * SKETCH_PIECES):
        put(N_SKETCH + i, rect(0, -SKETCH_THICK / 2, SKETCH_LENGTH, SKETCH_THICK / 2), MID)
    put(0, ribbon(), MID)     # the other lane's motion (build 060: unused on the variation page); the points page's line, first half
    put(2, ribbon(), MID)     # build 050: the other lane's second half on a wide window
    put(1, ribbon(), BRIGHT)  # the lane's; the line's second half
    put(3, ribbon(), BRIGHT)  # build 050: the lane's second half on a wide window
    # The playhead. Build 054 ("on the shape screen can we not just use the same playhead we're using on the
    # variation page"): a hairline the plot's height, as the cursor is, centred on the plot (it was a small capped
    # mark across the ruler). The shape page tells it from the edit cursor by its shade, white against the cursor's grey.
    put(5 + BAR_COUNT + TILE_COUNT, rect(-0.5, -CURSOR_HALF, 0.5, CURSOR_HALF), TEXT)
    put(n_cursor, rect(-0.5, -CURSOR_HALF, 0.5, CURSOR_HALF), TEXT)  # the points page's cursor, centred
    for i in range(POINT_COUNT):
        put(n_points + i, rect(-2, -2, 2, 2), BRIGHT)
    node = n_handles

    def bound(d, fill):
        nonlocal node
        put(node, d, fill)
        node += 1

    def circle(r, fill, stroke, width):
        nonlocal node
        add(f'<circle id="{ID}_node_{node}" cx="0" cy="0" r="{num(r)}" fill="{fill}" stroke="{stroke}" '
            f'stroke-width="{num(width)}"/>')
        node += 1

    # Build 021: the handles (build 024: 84-86), encoder 2's dot (87; 021's
    # encoder 4), the lane letters (88-91: a's ring and stem, b's ring and
    # stem; each ring before its stem, which draws over the ring's black fill);
    # build 022: encoder 1's dot (92). All centred on their origin.
    for _ in range(HANDLE_COUNT):
        circle(HANDLE_R, BLACK, BRIGHT, HANDLE_STROKE)
    circle(*ENC2)
    for stem in (STEM_A, STEM_B):
        circle(RING_R, BLACK, WHITE, 1)
        bound(rect(-STEM_W / 2, -stem / 2, STEM_W / 2, stem / 2), WHITE)
    circle(*ENC1)
    put(N_LETTER_C, letter_c(), WHITE)  # build 060: c, authored with the others' fill (the page paints it white or mid)
    assert node == n_grid
    node = n_grid + GRID_COUNT + AS_SET_COUNT
    # Build 052: the control strip's eight cells, black, placed at (60 i + 30, 10) in each column's ramp shade.
    # Build 054 ("drop the chevrons...that makes sense"): 052's four M2 and M3 chevrons are gone with them.
    # The - and + keys still step the variation, and the M keys carry words instead (the mark slots).
    for i in range(CELL_COUNT):
        put(N_CELLS + i, rect(-CELL_W / 2, -CELL_H / 2, CELL_W / 2, CELL_H / 2), BLACK)
    for i, (_, fill, size, x, y) in enumerate(TEXTS):
        add(f'<text id="{ID}_text_{i}" transform="matrix(1 0 0 1 {x} {y})" fill="{fill}" '
            f'font-family="{FONT}" font-size="{size}px">-</text>')
    for i, colour in enumerate(PALETTE):
        add(f'<circle id="{ID}_palette_{i}" cx="640" cy="0" r="1" fill="{colour}" stroke="{colour}" stroke-width="0.5"/>')
    for i, (fill, stroke, width) in enumerate(RINGS):
        add(f'<circle id="{ID}_palette_{len(PALETTE) + i}" cx="640" cy="0" r="1" fill="{fill}" stroke="{stroke}" '
            f'stroke-width="{num(width)}"/>')
    add(f'<circle id="{ID}_palette_{len(PALETTE) + len(RINGS)}" cx="640" cy="0" r="1" fill="{GRID}" stroke="{GRID}" stroke-width="0.5"/>')
    add(f'<circle id="{ID}_palette_{len(PALETTE) + len(RINGS) + 1}" cx="640" cy="0" r="1" fill="{BAND}" stroke="{BAND}" stroke-width="0.5"/>')
    for i, grey in enumerate(TIMELINE):
        add(f'<circle id="{ID}_palette_{len(PALETTE) + len(RINGS) + 2 + i}" cx="640" cy="0" r="1" fill="{grey}" stroke="{grey}" stroke-width="0.5"/>')
    add(f'<circle id="{ID}_palette_{len(PALETTE) + len(RINGS) + 2 + len(TIMELINE)}" cx="640" cy="0" r="1" fill="{RAMP}" stroke="{RAMP}" stroke-width="0.5"/>')
    add("</svg>")
    return "\n".join(out) + "\n", node


def main(argv):
    data, _ = artwork()
    if "--check" in argv:
        with open(OUT) as f:
            return 0 if f.read() == data else 1
    with open(OUT, "w") as f:
        f.write(data)
    print(f"{OUT}: {len(data)} bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
