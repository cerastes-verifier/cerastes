// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_default.c
// Pins: 0019 item 3: every path assigns, the default assigns the wrong value
#include <cerastes.h>

int grade(int code)
    CERASTES_REQUIRES(0 <= code && code <= 5)
    CERASTES_ENSURES(r, (code == 0 && r == 10) || (code == 1 && r == 20) || (code >= 2 && r == 0))
{
  int x;
  switch (code) {
  case 0:
    x = 10;
    break;
  case 1:
    x = 20;
    break;
  default:
    x = 1;
    break;
  }
  return x;
}
