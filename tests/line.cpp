// Host test: line.hpp against floating point (the mockup's valueAt).
#include "../line.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace variations;
static void require(bool ok, const char* why) {
    if (!ok) {
        std::fprintf(stderr, "%s\n", why);
        std::exit(1);
    }
}
static double ref_bend(double f, int c) { return std::pow(f, std::pow(2.0, 2.0 * c / 127.0)); }
int main() {
    // bend against pow, every curve, a sweep of f.
    double worst = 0;
    for (int c = -127; c <= 127; ++c)
        for (u32 f = 0; f <= kOne; f += 97) {
            const double got = bend(f, c) / 65536.0, want = ref_bend(f / 65536.0, c);
            worst = std::fmax(worst, std::fabs(got - want));
        }
    std::printf("bend: worst error %.5f\n", worst);
    require(worst < 0.004, "bend within 0.4% of pow");
    require(bend(0, 50) == 0 && bend(kOne, -50) == kOne && bend(12345, 0) == 12345, "ends and straight");
    for (int c = -127; c <= 127; c += 7) {
        u32 last = 0;
        for (u32 f = 0; f <= kOne; f += 256) {
            const u32 b = bend(f, c);
            require(b >= last, "monotonic");
            last = b;
        }
    }
    // value_at: holds, straight, curved, descending.
    const Point one[] = {{8, 100, 0}};
    require(value_at(one, 1, 0) == 100 << 8 && value_at(one, 1, 60000) == 100 << 8, "a single point holds");
    const Point line[] = {{0, 0, 0}, {31, 255, 0}, {40, 255, 0}, {63, 0, 90}};
    require(value_at(line, 4, 0) == 0, "start");
    require(value_at(line, 4, 31 * 480) == 255 << 8, "first end");
    const u32 mid = value_at(line, 4, 31 * 240);
    require(mid > 127 * 256 && mid < 128 * 256 + 1, "straight halfway");
    require(value_at(line, 4, 35 * 480) == 255 << 8, "flat between equal points");
    const u32 curved = value_at(line, 4, (40 * 480 + 63 * 480) / 2);
    const double want = 255 + (0 - 255) * ref_bend(0.5, 90);
    require(std::fabs(curved / 256.0 - want) < 1.5, "a curved descent");
    require(value_at(line, 4, 63 * 480) == 0 && value_at(line, 4, 99999) == 0, "holds after the last");
    u32 last = 0;
    for (u32 at = 0; at <= 31 * 480; at += 7) {
        const u32 v = value_at(line, 4, at);
        require(v >= last, "rises steadily");
        last = v;
    }
    std::puts("variations line: ok");
    return 0;
}
