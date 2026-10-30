// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 2: a loop condition that reads a cell through a pointer (the top-test shape: its checks run at the top of the body), a measure and an invariant over the cell
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x = 0;
  int *p = &x;
  CERASTES_INVARIANT(0 <= x && x <= n)
  CERASTES_DECREASES(n - x)
  while (*p < n) {
    *p += 1;
  }
  return x;
}
