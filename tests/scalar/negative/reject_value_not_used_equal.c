// expect: rejected cerastes-value_not_used @ L9
// mode: c23, c++20
// twin: tests/scalar/incdec_value.c
// Pins: 0020 item 5: a discarded expression whose outermost operator is not an effect, with a side effect inside, is rejected
#include <cerastes.h>

int f(int x, int y)
{
  x == (y = 1);
  return x + y;
}
