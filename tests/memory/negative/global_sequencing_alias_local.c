// expect: error unsequenced @ L14
// mode: c23, c++20
// twin: tests/memory/global_sequencing.c
// Pins: 0041 item 11: p == &g, so g++ + *p modifies and reads g with no sequence point between: unsequenced (the path-precise p != &g fails)
#include <cerastes.h>

static int g = 3;

int alias(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g && g < 100)
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  int *p = &g;
  return g++ + *p;
}
