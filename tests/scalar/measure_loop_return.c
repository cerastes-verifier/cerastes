// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0013 item 8: a loop left by return still has to terminate (the exit is a goto out of the loop)
#include <cerastes.h>

int first_big(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r >= 0 && r <= n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i >= 50) {
      return i;
    }
    i += 1;
  }
  return i;
}
