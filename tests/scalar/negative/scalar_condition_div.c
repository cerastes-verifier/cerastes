// expect: error division-by-zero @ L18
// mode: c23, c++20
// twin: tests/scalar/scalar_condition.c
// Pins: 0010 item 6: without the requires, x may be 0
#include <cerastes.h>

int nonzero(int x)
    CERASTES_ENSURES(r, r == (x != 0))
{
  if (x) {
    return 1;
  }
  return 0;
}

int one_over(int x)
{
  return 100 / x;
}
