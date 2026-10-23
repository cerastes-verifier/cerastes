// expect: rejected cerastes-comma_without_effect @ L12
// mode: c23, c++20
// twin: tests/scalar/comma_for.c
// Pins: 0020 item 5: the right operand of a for init comma has no effect
#include <cerastes.h>

int f(int a)
{
  int x = 0;
  CERASTES_INVARIANT(0 <= x && x <= 3)
  CERASTES_DECREASES(3 - x)
  for (x = 0, a; x < 3; x++) {
  }
  return x + a;
}
