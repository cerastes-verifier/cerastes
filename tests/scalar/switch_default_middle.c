// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: default may be anywhere; the dispatch tries the values first; default falls into the next case
#include <cerastes.h>

int rate(int x)
    CERASTES_ENSURES(r, (x == 1 && r == 10) || (x == 2 && r == 20) || (x != 1 && x != 2 && r == 21))
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
