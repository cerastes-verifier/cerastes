// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 7: a break out of a switch whose body declares a cell (the shape that 0032 item 10 rejected) and one out of a block inside a case: the break exits only the blocks nested in the body
#include <cerastes.h>

int switch_cell(int code)
    CERASTES_ENSURES(r, (code == 0 && r == 5) || (code == 1 && r == 3) || (code != 0 && code != 1 && r == 1))
{
  int total = 0;
  switch (code) {
    int x;
    int *p;
  case 0:
    p = &x;
    *p = 5;
    total = x;
    break;
  case 1: {
    int y = 3;
    int *q = &y;
    total = *q;
    break;
  }
  default:
    total = 1;
  }
  return total;
}
