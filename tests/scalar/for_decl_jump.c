// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 items 2 and 4: break and continue in a for loop that declares its variable; continue runs the step
// The loop variable cannot be named in the annotations, so the invariant and measure use the outer c; k is read (k - k) so that no compiler warns.
#include <cerastes.h>

int count_for(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (n < 3 && r == n) || (3 <= n && n <= 6 && r == n - 1) || (n >= 7 && r == 6))
{
  int c = 0;
  int s = 0;
  CERASTES_INVARIANT(c <= n && c <= 6 && ((c < 3 && s == c) || (c >= 3 && s == c - 1)))
  CERASTES_DECREASES(n - c)
  for (int k = 0; c < n; k = c) {
    c += 1 + (k - k);
    if (c == 3) {
      continue;
    }
    s += 1;
    if (c == 7) {
      break;
    }
  }
  return s;
}
