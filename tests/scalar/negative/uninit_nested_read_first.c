// expect: error uninitialized @ L12
// mode: c23, c++20
// twin: tests/scalar/uninit_nested_assign.c
// Pins: 0019 item 13: x is read before the assignment that gives it a value
#include <cerastes.h>

int nested(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  int x;
  int y = x + 1;
  x = a;
  return y + x;
}
