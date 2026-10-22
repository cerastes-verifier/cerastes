// expect: error uninitialized @ L14
// mode: c23, c++20
// twin: tests/scalar/uninit_conditional_arm.c
// Pins: 0019 item 2: the condition admits c == 0, where x was not assigned
#include <cerastes.h>

int pick(int c, int a)
    CERASTES_ENSURES(r, (c > 0 && r == a) || c <= 0)
{
  int x;
  if (c > 0) {
    x = a;
  }
  return c >= 0 ? x : 0;
}
