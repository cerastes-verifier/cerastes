// expect: rejected cerastes-unsupported_expression @ L16
// mode: c23, c++20
// twin: tests/scalar/call_precondition.c
// Pins: 0012 item 6: a call in a guarded operand of && is rejected
// A hoisted call would run when C/C++ does not run it.
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  return x;
}

int test(int a)
{
  return a > 0 && pos(a) > 1;
}
