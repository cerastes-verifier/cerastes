// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 7: == and != compare pointers; the addresses of two objects differ (checked in a check)
#include <cerastes.h>

int addresses(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int n = 0;
  CERASTES_CHECK(&x != &y);
  if (p == &x) {
    n += 1;
  }
  if (p != &y) {
    n += 1;
  }
  return n;
}
