// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/scalar/switch_gnu_fallthrough.c
// Pins: 0014 item 10: the gnu::fallthrough falls into the next label
#include <cerastes.h>

int bonus(int k)
    CERASTES_REQUIRES(0 <= k && k <= 2)
    CERASTES_ENSURES(r, (k == 0 && r == 4) || (k == 1 && r == 2) || (k == 2 && r == 2))
{
  int total = 0;
  switch (k) {
  case 0:
    total += 4;
    [[gnu::fallthrough]];
  default:
    total += 2;
    break;
  }
  return total;
}
