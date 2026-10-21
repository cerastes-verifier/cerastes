// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: a case without break falls into the next label ([[fallthrough]])
#include <cerastes.h>

int bonus(int k)
    CERASTES_REQUIRES(0 <= k && k <= 3)
    CERASTES_ENSURES(r, (k == 0 && r == 6) || (k == 1 && r == 2) || (k == 2 && r == 1) || (k == 3 && r == 100))
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
