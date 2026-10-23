// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 2: the step of a for loop runs after continue; here every path assigns x before the step
#include <cerastes.h>

int steps(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1073741823)
    CERASTES_ENSURES(r, r == n + n - (n >= 3 ? 1 : 0))
{
  int x;
  int total = 0;
  int hits = 0;
  int i = 0;
  CERASTES_INVARIANT(i <= n && total == i && hits == i - (i >= 3 ? 1 : 0))
  CERASTES_DECREASES(n - i)
  for (; i < n; total = x) {
    i += 1;
    x = i;
    if (i == 3) {
      continue;
    }
    hits += 1;
  }
  return total + hits;
}
