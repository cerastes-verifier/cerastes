// expect: error overflow @ L18
// mode: c23, c++20
// twin: tests/scalar/call_unsequenced_sum.c
// Pins: 0021: the sum of two unsequenced calls is checked after both; the callee no longer caps its result, so the sum may overflow
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int sum_two(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r >= 0)
{
  return bounded(a) + bounded(b);
}
