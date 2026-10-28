// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 4, 10: a cell that the loop does not touch stays live across it (the loop has a break)
#include <cerastes.h>

int across(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 50) {
      break;
    }
    i += 1;
  }
  return *p;
}
