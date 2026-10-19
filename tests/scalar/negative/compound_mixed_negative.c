// expect: error conversion @ L12
// mode: c23, c++20
// twin: tests/scalar/compound_mixed.c
// Pins: 0010 item 6: -5 + 0u is 4294967291 in unsigned, which does not fit int again
// The sum is computed in unsigned; converting it back to int is the checked step.
#include <cerastes.h>

int add_unsigned(int i, unsigned u)
    CERASTES_REQUIRES(i >= -5 && i <= 1000 && u <= 1000u)
    CERASTES_ENSURES(r, r >= 0)
{
  i += u;
  return i;
}
