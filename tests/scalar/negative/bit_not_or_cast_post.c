// expect: error postcondition @ L14
// expect: error postcondition @ L22
// expect: error postcondition @ L29
// mode: c23, c++20
// twin: tests/scalar/bit_not_or_cast.c
// Pins: 0023 item 5: the values of !x | (int)(y > 0) and !x | 1L are exact
#include <cerastes.h>

int or_cast(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int y = 5;
  return !x | (int)(y > 0);
}

int or_cast_zero(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 3;
  int y = -5;
  return !x | (int)(y > 0);
}

int or_one_long(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 4;
  return !x | 1L;
}
