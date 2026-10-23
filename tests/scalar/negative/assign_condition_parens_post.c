// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/assign_condition_parens.c
// Pins: 0020 item 4: the condition's assignment stores a - 1
#include <cerastes.h>

int clamp(int a)
    CERASTES_REQUIRES(-2147483647 <= a)
    CERASTES_ENSURES(r, r == (a - 1 > 0 ? a - 1 : -1))
{
  int x;
  if ((x = a - 1) > 0) {
    return x;
  }
  return 0;
}

int nonzero(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? a : -1))
{
  int x;
  if ((x = a) != 0) {
    return x;
  }
  return -1;
}
