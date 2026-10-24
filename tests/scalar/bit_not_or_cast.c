// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 5: !x | (int)(y > 0) is silent in every compiler (an explicit cast of a truth value as the right operand of |), and so is !x | 1L
#include <cerastes.h>

// The result comes from !x alone: the cast is 0.
int or_cast(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int y = 0;
  return !x | (int)(y > 0);
}

// Both operands are 0.
int or_cast_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 1;
  int y = 0;
  return !x | (int)(y > 0);
}

// The result comes from the cast alone: !x is 0, y is just above the boundary of y > 0.
int or_cast_one(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 3;
  int y = 1;
  return !x | (int)(y > 0);
}

int or_one_long(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 4;
  return !x | 1L;
}
