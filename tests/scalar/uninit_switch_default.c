// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 3: every case and the default assign
#include <cerastes.h>

int grade(int code)
    CERASTES_ENSURES(r, r == (code == 0 ? 10 : (code == 1 ? 20 : 0)))
{
  int x;
  switch (code) {
  case 0:
    x = 10;
    break;
  case 1:
    x = 20;
    break;
  default:
    x = 0;
  }
  return x;
}
