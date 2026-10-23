// expect: error precondition @ L18
// mode: c23, c++20
// twin: tests/scalar/call_unsequenced_sum.c
// Pins: 0021: the precondition of the second of two unsequenced calls is checked (b may be -1)
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == (x > 1000 ? 1000 : x))
{
  return x > 1000 ? 1000 : x;
}

int sum_two(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= -1)
    CERASTES_ENSURES(r, r == (a > 1000 ? 1000 : a) + (b > 1000 ? 1000 : b))
{
  return bounded(a) + bounded(b);
}
