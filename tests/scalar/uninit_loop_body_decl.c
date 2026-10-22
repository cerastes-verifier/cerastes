// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 items 2 and 4: a local declared in a loop body is indeterminate again in each iteration, and assigned before its read
#include <cerastes.h>

int sum_threes(int n)
    CERASTES_REQUIRES(0 <= n && n <= 715827882)
    CERASTES_ENSURES(r, r == 3 * n)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(i <= n && s == 3 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int t;
    t = 3;
    s += t;
    i += 1;
  }
  return s;
}
