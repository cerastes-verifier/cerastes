// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/global_two_atoms.c
// Pins: 0041 item 8: the body leaves g == 3, the ensures say g == 2
#include <cerastes.h>

static int g = 3;

void two(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && g == 2)
{
  *p = 1;
  g = 3;
}

int main(void)
{
  return g - g;
}
