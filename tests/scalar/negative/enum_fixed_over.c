// expect: error conversion @ L13
// mode: c23, c++20
// twin: tests/scalar/enum_fixed.c
// Pins: 0018 item 5: 256 is not one of the values 0 to 255 of enum small: an error, never a wrap
#include <cerastes.h>

enum small : unsigned char { S0, S1 };

enum small from_int(int x)
    CERASTES_REQUIRES(0 <= x && x <= 256)
    CERASTES_ENSURES(r, (int)r == x)
{
  enum small s = (enum small)x;
  return s;
}

int index_of(enum small s)
    CERASTES_ENSURES(r, r == (int)s)
{
  return (int)s;
}

int use(void)
    CERASTES_ENSURES(r, r == 256)
{
  enum small lo = from_int(0);
  enum small hi = from_int(255);
  int a = index_of(lo);
  int b = index_of(hi);
  int c = index_of(S1);
  return a + b + c;
}
