// expect: rejected cerastes-side_effect_not_evaluated @ L10
// mode: c23, c++20
// twin: tests/scalar/incdec_value.c
// Pins: 0020 item 5: a side effect in the operand of a sizeof inside a sizeof is never evaluated
#include <cerastes.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  int n = (int)sizeof(sizeof(x++));
  return n + x;
}
