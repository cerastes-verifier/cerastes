// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/global_two_atoms.c
// Pins: 0041 item 8 (the aliasing trap): two(&g) passes g's one cell as *p and as g: the separation p != &g fails at the call
#include <cerastes.h>

static int g = 3;

void two(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && g == 2)
{
  *p = 1;
  g = 2;
}

int main(void)
{
  two(&g);
  return 0;
}
