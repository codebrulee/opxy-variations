# Page fuzzers

Random page input against `draw::valid`, which the device enforces on every frame: an
invalid frame draws nothing, which is how the page froze in build 052. Each takes the
number of inputs then a seed.

```sh
g++ -std=c++17 -O1 -fsanitize=address,undefined \
    -I../../../open-xy-vdev/sdk/include -o /tmp/fuzz_deep fuzz_deep.cpp
/tmp/fuzz_deep 60000 7
```

| | The song it drives | Why |
| --- | --- | --- |
| `fuzz_page.cpp` | no song shape | the original, build 052 |
| `fuzz_shape.cpp` | 4 entries, page at entry 1 | a shape, so the band, markers and length list run |
| `fuzz_deep.cpp` | 12 entries, page at entry 6 | build 058's backward reach, which `fuzz_shape` only reached one entry deep |
| `fuzz_long.cpp` | 96 entries of 16-64 steps, page at entry 48 | build 063's version 12 ceilings; a third argument biases it toward growing the window, without which it only reached 812 steps |

Run them after any change to the page's drawing or its bounds. They have caught nothing
since build 053's invalid frame, but that is the failure they exist for: the host tests
assert what a frame contains, and these assert that a frame is drawable at all.
