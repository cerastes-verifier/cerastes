// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: a break in an inner switch leaves the inner switch only; the outer case goes on
#include <cerastes.h>

int nested(int a, int b)
    CERASTES_ENSURES(r, (a == 0 && b == 0 && r == 11) || (a == 0 && b == 1 && r == 12) || (a == 0 && b != 0 && b != 1 && r == 10) || (a == 1 && r == 20) || (a != 0 && a != 1 && r == 99))
{
  int total = 0;
  switch (a) {
  case 0:
    switch (b) {
    case 0:
      total += 1;
      break;
    case 1:
      total += 2;
      break;
    }
    total += 10;
    break;
  case 1:
    total = 20;
    break;
  default:
    total = 99;
    break;
  }
  return total;
}
