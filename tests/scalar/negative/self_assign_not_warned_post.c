// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8: the accepted shapes are verified, not skipped: x += x doubles x, so the result is not 2 * x + 1
#include <cerastes.h>

int doubled(int x)
    CERASTES_REQUIRES(-1073741824 <= x && x <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * x + 1)
{
  x += x;
  return x;
}
