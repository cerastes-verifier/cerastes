// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/scalar/switch_default_middle.c
// Pins: 0014 item 6: the default in the middle falls into case 2
// Claiming r == 1 for the default forgets the fallthrough into case 2.
#include <cerastes.h>

int rate(int x)
    CERASTES_ENSURES(r, (x == 1 && r == 10) || (x == 2 && r == 20) || (x != 1 && x != 2 && r == 1))
{
  int total = 0;
  switch (x) {
  case 1:
    total = 10;
    break;
  default:
    total = 1;
    [[fallthrough]];
  case 2:
    total += 20;
    break;
  }
  return total;
}
