// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/global_address.c
// Pins: 0041 item 6: p may point elsewhere than at g, so r == 1 is not a postcondition of p == &g
#include <cerastes.h>

static int g = 3;

int is_g(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 1)
{
  return p == &g;
}
