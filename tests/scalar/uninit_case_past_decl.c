// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 6: a case label may follow a declaration without an initializer, at the top of the switch body or between cases
#include <cerastes.h>

int top_decl(int code)
    CERASTES_ENSURES(r, r == (code == 0 ? 1 : (code == 1 ? 5 : 6)))
{
  int total;
  switch (code) {
    int x;
  case 0:
    total = 1;
    break;
  case 1:
    x = 5;
    total = x;
    break;
  default:
    x = 6;
    total = x;
  }
  return total;
}

int mid_decl(int code)
    CERASTES_ENSURES(r, r == (code == 0 ? 2 : (code == 1 ? 9 : 0)))
{
  int total = 0;
  switch (code) {
  case 0:
    total = 2;
    break;
    int y;
  case 1:
    y = 9;
    total = y;
  }
  return total;
}
