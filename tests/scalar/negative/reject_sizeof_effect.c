// expect: rejected cerastes-side_effect_not_evaluated @ L10
// mode: c23, c++20
// twin: tests/scalar/incdec_value.c
// Pins: 0020 item 4: a side effect in an unevaluated operand (sizeof(x++)) is rejected
#include <cerastes.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  int n = (int)sizeof(x++);
  return n + x;
}
