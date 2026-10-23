// expect: rejected cerastes-value_not_used @ L10
// mode: c23, c++20
// twin: tests/scalar/incdec_value.c
// Pins: 0020 item 4: a discarded operator with an effect inside (x++ + 1;) is rejected
#include <cerastes.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  x++ + 1;
  return x;
}
