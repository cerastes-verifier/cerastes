// expect: rejected cerastes-unsupported_expression @ L15
// mode: c23, c++20
// twin: tests/scalar/call_precondition.c
// Pins: 0012 item 6: a call in a branch of ?: is rejected
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  return x;
}

int test(int a)
{
  return a > 0 ? pos(a) : 0;
}
