// expect: error postcondition @ L27
// mode: c23, c++20
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 6: the fallthrough adds the next case's effect
// Claiming r == 4 for k == 0 forgets that case 0 falls into case 1.
#include <cerastes.h>

int bonus(int k)
    CERASTES_REQUIRES(0 <= k && k <= 3)
    CERASTES_ENSURES(r, (k == 0 && r == 4) || (k == 1 && r == 2) || (k == 2 && r == 1) || (k == 3 && r == 100))
{
  int total = 0;
  switch (k) {
  case 0:
    total += 4;
    [[fallthrough]];
  case 1:
    total += 2;
    break;
  case 2:
    total += 1;
    break;
  default:
    total = 100;
    break;
  }
  return total;
}
