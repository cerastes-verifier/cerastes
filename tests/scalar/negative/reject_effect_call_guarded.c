// expect: rejected cerastes-unsupported_expression @ L17
// mode: c23, c++20
// twin: tests/scalar/and_assign.c
// Pins: 0020 (until M4.3b): an effect with a call in the guarded operand of && is rejected
#include <cerastes.h>

int pos(int v)
    CERASTES_REQUIRES(v > 0)
{
  return v;
}

int f(int a)
    CERASTES_REQUIRES(0 < a && a < 1000)
{
  int x = 0;
  if (a > 0 && (x = pos(a)) > 5) {
    return x;
  }
  return 0;
}
