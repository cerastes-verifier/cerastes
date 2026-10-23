// expect: rejected cerastes-value_not_used @ L10
// mode: c23, c++20
// twin: tests/scalar/discarded_guarded_effects.c
// Pins: 0020 item 5: a discarded expression whose outermost operator is not an effect, with a side effect inside, is rejected like -Wunused-value does
#include <cerastes.h>

int f(int a, int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  a && x++;
  return x;
}
