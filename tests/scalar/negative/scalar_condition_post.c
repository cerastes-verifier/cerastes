// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/scalar_condition.c
// Pins: 0010 item 6: the second return is the x == 0 case
#include <cerastes.h>

int nonzero(int x)
    CERASTES_ENSURES(r, r == 1)
{
  if (x) {
    return 1;
  }
  return 0;
}

int one_over(int x)
    CERASTES_REQUIRES(x)
{
  return 100 / x;
}
