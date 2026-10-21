// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/do_zero_expr.c
// Pins: 0014 item 5: the exact ensures of the positive notices a wrong quotient
// The body computes 9 / x, so the result is not the 10 / x the ensures pins.
#include <cerastes.h>

int ten_over(int x)
    CERASTES_ENSURES(r, (x == 0 && r == 0) || (x != 0 && r == 10 / x))
{
  int y = 0;
  do {
    if (x == 0) {
      continue;
    }
    y = 9 / x;
  } while (1 - 1);
  return y;
}
