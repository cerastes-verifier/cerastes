// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 8: two loops in a row, each with its own invariant
#include <cerastes.h>

int count_both(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 1000)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    total += 1;
  }
  int j = 0;
  CERASTES_INVARIANT(0 <= j && j <= n && total == n + j)
  CERASTES_DECREASES(n - j)
  while (j < n) {
    j += 1;
    total += 1;
  }
  return total;
}
