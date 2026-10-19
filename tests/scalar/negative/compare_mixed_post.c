// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/compare_mixed.c
// Pins: 0010 item 6: naive mathematics (-1 + 0 > 0 is false) is not C
#include <cerastes.h>

int mixed_above_zero(int i)
    CERASTES_REQUIRES(i == -1)
    CERASTES_ENSURES(r, r == 0)
{
  return i + 0u > 0u;
}
