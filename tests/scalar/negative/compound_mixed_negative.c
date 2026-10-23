// expect: error conversion @ L12
// mode: c23, c++20
// twin: tests/scalar/compound_mixed.c
// Pins: 0010 item 6: i += u is computed in unsigned, and the unsigned sum may not fit int again
// The sum is computed in unsigned (it fits, u is at most UINT_MAX - 1000); converting it back to int is the checked step.
#include <cerastes.h>

int add_unsigned(int i, unsigned u)
    CERASTES_REQUIRES(i >= 0 && i <= 1000 && u <= 4294966295u)
    CERASTES_ENSURES(r, r >= 0)
{
  i += u;
  return i;
}
