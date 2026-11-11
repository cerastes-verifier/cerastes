// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 6 (0032 item 8): a break out of the array's block in a switch case runs the scope exit at the break
// Pre-registered from the M4.5e1a sweep (report 5, s12): verifies
#include <cerastes.h>

int pick(int c)
    CERASTES_ENSURES(r, r == (c == 1 ? 0 : 2))
{
  int v = 0;
  switch (c) {
  case 1: {
    int a[2] = {1, 2};
    if (a[0] == 1) {
      break;
    }
    v = a[1];
    break;
  }
  default:
    v = 2;
    break;
  }
  return v;
}
