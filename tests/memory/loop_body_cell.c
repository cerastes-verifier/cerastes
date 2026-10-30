// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 2 and 7: a cell declared in the loop body is a new block on each iteration; a continue leaves it (scope exit at the jump)
#include <cerastes.h>

int body_cells(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int total = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int x = 1;
    int *p = &x;
    i += 1;
    if (i == 7) {
      total += *p;
      continue;
    }
    total += *p;
  }
  return total;
}
