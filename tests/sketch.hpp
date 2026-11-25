// Build 060 (stage B): the helpers the page tests share for the sketched lanes (page.hpp n_sketch): the lanes not shown
// are drawn from sketch_pieces nodes each, in the order of the lanes left after the one shown.
#pragma once
#include "../page.hpp"

namespace sketch_check {
using namespace variations;
using opxy::i32;
using opxy::u32;
namespace draw = opxy::draw;

// Where lane `lane`'s sketch starts when `shown` is the lane shown (-1: it is the lane shown, it has none).
inline i32 first_node(u32 shown, u32 lane) {
    if (lane == shown) return -1;
    return static_cast<i32>(screen::n_sketch + (lane < shown ? lane : lane - 1) * screen::sketch_pieces);
}
template <class F> bool is_shown(const F& frame, u32 node) { return frame.nodes[node].x != draw::hidden_x * draw::unit; }
// How many of lane `lane`'s pieces are drawn (all of them, or none).
template <class F> u32 pieces_shown(const F& frame, u32 shown, u32 lane) {
    const i32 first = first_node(shown, lane);
    u32 n = 0;
    if (first < 0) return 0;
    for (u32 k = 0; k < screen::sketch_pieces; ++k) n += is_shown(frame, static_cast<u32>(first) + k);
    return n;
}
// Every drawn piece of the lane is in shade `shade`.
template <class F> bool pieces_shade(const F& frame, u32 shown, u32 lane, u32 shade) {
    const i32 first = first_node(shown, lane);
    if (first < 0) return true;
    for (u32 k = 0; k < screen::sketch_pieces; ++k)
        if (is_shown(frame, static_cast<u32>(first) + k) && frame.nodes[static_cast<u32>(first) + k].shade != shade) return false;
    return true;
}
// All the sketch nodes drawn, over every lane.
template <class F> u32 all_shown(const F& frame) {
    u32 n = 0;
    for (u32 k = 0; k < screen::sketch_lanes * screen::sketch_pieces; ++k) n += is_shown(frame, screen::n_sketch + k);
    return n;
}
}  // namespace sketch_check
