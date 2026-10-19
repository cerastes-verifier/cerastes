// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/compound_ops.c
// Pins: 0012 item 7: (x + 1) * 2 - 2 is 2x, not 3x
#include <cerastes.h>

int mix(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 1000)
    CERASTES_ENSURES(r, r == x * 3)
{
  x += 1;
  x *= 2;
  x -= 2;
  return x;
}
