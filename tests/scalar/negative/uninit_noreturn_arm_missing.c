// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/scalar/uninit_noreturn_arm.c
// Pins: 0019 item 3: the other arm does nothing, so a < 0 reaches the read
#include <cerastes.h>

int non_negative(int a)
    CERASTES_ENSURES(r, a < 0 || r == a)
{
  int x;
  if (a >= 0) {
    x = a;
  } else {
  }
  return x;
}
