// expect: error uninitialized @ L14
// mode: c23, c++20
// twin: tests/scalar/uninit_if_else.c
// Pins: 0019 item 3: one arm assigns, so the read at the return may see an indeterminate value (a <= 0)
#include <cerastes.h>

int sign(int a)
    CERASTES_ENSURES(r, (a > 0 && r == 1) || a <= 0)
{
  int x;
  if (a > 0) {
    x = 1;
  }
  return x;
}
