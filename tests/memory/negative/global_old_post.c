// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/global_old_decreases.c
// Pins: 0041 item 7: old(g) is g's entry value: g == old(g) fails after g += 1
#include <cerastes.h>

static int g = 5;

void inc(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g == cerastes_old(g))
{
  g += 1;
}
