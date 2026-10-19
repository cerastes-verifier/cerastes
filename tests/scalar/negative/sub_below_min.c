// expect: error overflow @ L12
// mode: c23, c++20
// twin: tests/scalar/sub_bounds.c
// Pins: 0010 item 3: signed subtraction may go below INT_MIN
// a may be INT_MIN and b 1.
#include <cerastes.h>

int sub(int a, int b)
    CERASTES_REQUIRES(b >= 0)
    CERASTES_ENSURES(r, r <= a)
{
  return a - b;
}
