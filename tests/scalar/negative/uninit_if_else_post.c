// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/uninit_if_else.c
// Pins: 0019 item 3: both arms assign, one of them the wrong value
#include <cerastes.h>

int sign(int a)
    CERASTES_ENSURES(r, (a > 0 && r == 1) || (a <= 0 && r == -1))
{
  int x;
  if (a > 0) {
    x = 1;
  } else {
    x = 0;
  }
  return x;
}
