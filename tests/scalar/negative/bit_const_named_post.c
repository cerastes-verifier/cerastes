// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/bit_const_named.c
// Pins: 0023 item 6: AB is A | B = 3, not 2
#include <cerastes.h>

enum { A = 1, B = 2, AB = A | B, AX = AB ^ A, NA = ~A };

int ab_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return AB;
}

int ax_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return AX;
}

int na_value(void)
    CERASTES_ENSURES(r, r == -2)
{
  return NA;
}
