// expect: error division-by-zero @ L13
// mode: c23, c++20
// twin: tests/scalar/do_zero_expr.c
// Pins: 0014 item 5: the body of the exempt loop is verified (a division by zero in it)
// Without the guarding continue, x == 0 reaches the division.
#include <cerastes.h>

int ten_over(int x)
    CERASTES_ENSURES(r, (x == 0 && r == 0) || (x == 1 && r == 10) || (x != 0 && x != 1 && r >= 0 - 10 && r <= 10))
{
  int y = 0;
  do {
    y = 10 / x;
  } while (1 - 1);
  return y;
}
