// expect: error uninitialized @ L20
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_default.c
// Pins: 0019 item 3: without a default, code >= 2 leaves x unassigned
#include <cerastes.h>

int grade(int code)
    CERASTES_REQUIRES(0 <= code && code <= 5)
    CERASTES_ENSURES(r, (code == 0 && r == 10) || (code == 1 && r == 20) || code >= 2)
{
  int x;
  switch (code) {
  case 0:
    x = 10;
    break;
  case 1:
    x = 20;
    break;
  }
  return x;
}
