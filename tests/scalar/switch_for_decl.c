// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: a for loop that declares its variable directly in a case (no braces) is accepted
#include <cerastes.h>

int fill(int mode, int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (mode == 1 && r == n) || (mode == 0 && r == 7) || (mode != 0 && mode != 1 && r == 9))
{
  int c = 0;
  switch (mode) {
  case 1:
    CERASTES_INVARIANT(c <= n)
    CERASTES_DECREASES(n - c)
    for (int k = 0; c < n; k = c) {
      c += 1 + (k - k);
    }
    break;
  case 0:
    c = 7;
    break;
  default:
    c = 9;
    break;
  }
  return c;
}
