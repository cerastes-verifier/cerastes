// expect: error unsequenced @ L21
// mode: c23, c++20
// twin: tests/memory/global_sequencing.c
// Pins: 0041 item 11 (0025 item 2, 0028 item 13): g + zero() with zero owning g: the call is a memory call on g's cell and C does not order it with the read (the places are equal)
#include <cerastes.h>

static int g = 3;

int zero(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(g) && g == 0 && r == 0)
{
  g = 0;
  return 0;
}

int use(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  return g + zero();
}
