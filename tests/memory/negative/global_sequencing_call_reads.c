// expect: error unsequenced @ L20
// mode: c23, c++20
// twin: tests/memory/global_sequencing.c
// Pins: 0041 item 11: g++ + look() with look reading g: the write and the call that reads the same cell are unordered
#include <cerastes.h>

static int g = 3;

int look(void)
    CERASTES_REQUIRES(cerastes_reads(g) && 0 <= g && g < 100)
    CERASTES_ENSURES(r, cerastes_reads(g) && r == g)
{
  return g;
}

int use(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g && g < 50)
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  return g++ + look();
}
