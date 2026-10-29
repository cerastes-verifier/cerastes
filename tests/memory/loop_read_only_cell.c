// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3: a cell the loop only reads keeps its value (no write can reach it), so the ensures after the loop holds
#include <cerastes.h>

int keeps(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*p == 7) {
      i += 1;
    } else {
      i += 1;
    }
  }
  return x;
}
