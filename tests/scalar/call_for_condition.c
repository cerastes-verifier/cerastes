// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: a call in a for condition runs at every evaluation, after the step; its precondition holds at each
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && r > 0)
{
  return x + 1;
}

int count_for(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == i)
  CERASTES_DECREASES(n - i)
  for (; next(i) <= n; i++) {
    total += 1;
  }
  return total;
}
