// expect: error uninitialized @ L11
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_default.c
// Pins: 0019 item 2: the controlling expression of a switch is a read
#include <cerastes.h>

int dispatch(int a)
{
  int x;
  int total = 0;
  switch (x) {
  case 0:
    total = a;
    break;
  default:
    break;
  }
  return total;
}
