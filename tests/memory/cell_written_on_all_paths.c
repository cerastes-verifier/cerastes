// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 and 0032 items 6, 8: a cell written on every path is read at the return
#include <cerastes.h>

int magnitude(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
    CERASTES_ENSURES(r, r >= 0 && (r == a || r == -a))
{
  int x;
  int *p = &x;
  if (a > 0) {
    *p = a;
  } else {
    *p = -a;
  }
  return x;
}
