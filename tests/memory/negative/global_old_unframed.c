// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/global_old_decreases.c
// Pins: 0041 item 7 (0037 item 7): the clause reads g before the owns(g) that frames it: permission at the clause
#include <cerastes.h>

static int g = 5;

void inc(void)
    CERASTES_REQUIRES(g < 100 && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
}

int main(void)
{
  return g - g;
}
