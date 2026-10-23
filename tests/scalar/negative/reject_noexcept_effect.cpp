// expect: rejected cerastes-side_effect_not_evaluated @ L10
// mode: c++20
// twin: tests/scalar/typeof_effect.c
// Pins: 0020 item 5: a side effect in the operand of noexcept is never evaluated: rejected (C++ only)
#include <cerastes.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  int n = noexcept(x++) ? 1 : 0;
  return n + x;
}
