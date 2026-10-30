// expect: error pointer-comparison @ L13
// mode: c23, c++20
// twin: tests/memory/global_address.c
// Pins: 0041 item 6 (0037 item 12): live(p) alone allows a pointer one past its object, which may equal &g: pointer-comparison (the disjunct is about liveness, not about the comparison)
#include <cerastes.h>

static int g = 3;

int is_g(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(p))
{
  return p == &g;
}
