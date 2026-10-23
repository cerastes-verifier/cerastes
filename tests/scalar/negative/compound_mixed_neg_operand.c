// expect: error conversion @ L12
// mode: c23, c++20
// twin: tests/scalar/compound_mixed.c
// Pins: 0022 item 1: i += u converts i = -5 to unsigned first, which does not fit (the usual arithmetic conversions)
// The failing obligation is the conversion of i to unsigned int, before the sum exists.
#include <cerastes.h>

int add_unsigned(int i, unsigned u)
    CERASTES_REQUIRES(i >= -5 && i <= 1000 && u <= 1000u)
    CERASTES_ENSURES(r, r >= 0)
{
  i += u;
  return i;
}
