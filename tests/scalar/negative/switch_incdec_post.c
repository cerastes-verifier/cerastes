// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/scalar/switch_incdec.c
// Pins: 0020 item 1: the default case sees the incremented x
#include <cerastes.h>

int dispatch(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == (x == 0 ? 1 : (x == 1 ? 20 : -(x + 1))))
{
  switch (x++) {
  case 0:
    return x;
  case 1:
    return x * 10;
  default:
    return x;
  }
}
