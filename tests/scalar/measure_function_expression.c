// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0013 item 7: a function measure's arithmetic is checked in the well-formedness method, under the requires
#include <cerastes.h>

int down(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 1000)
    CERASTES_DECREASES(n * 1000)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return down(n - 1);
}
