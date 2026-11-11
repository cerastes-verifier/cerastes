// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 6 (0019 item 6): an array declared without an initializer before the first case label: its block is entered before the dispatch, its elements are written in the case that uses them
// Pre-registered from the M4.5e1a sweep (report 5, s13 plain): verifies
#include <cerastes.h>

int pick(int c)
    CERASTES_ENSURES(r, r == (c == 1 ? 1 : 0))
{
  switch (c) {
    int a[2];
  case 1:
    a[0] = 1;
    return a[0];
  default:
    return 0;
  }
}
