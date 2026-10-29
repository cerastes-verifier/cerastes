// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3 (S28 F3): a return inside a loop that names no memory while a &param cell is live: the loop holds the cell so the return's scope exit passes
#include <cerastes.h>

int pass_through(int a, int n)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, 0 <= r && r <= 100)
{
  int *pa = &a;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      return i;
    }
    i += 1;
  }
  return *pa;
}
