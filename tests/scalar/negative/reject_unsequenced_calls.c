// expect: rejected cerastes-unsupported_expression @ L19
// mode: c23, c++20
// twin: tests/scalar/call_in_expression.c
// Pins: 0012 item 6: two calls in one expression that are not nested in each other are unsequenced and rejected
// A callee that never returns would hide every check placed after it, although C and C++ may run those first.
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r >= 0 && r <= 1000)
{
  return x > 1000 ? 1000 : x;
}

int sum_two(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r >= 0)
{
  return bounded(a) + bounded(b);
}
