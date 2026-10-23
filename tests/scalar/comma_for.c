// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2: a for loop's init and step clauses are comma expressions
#include <cerastes.h>

int meet(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == (n + 1) / 2)
{
  int i, j;
  CERASTES_INVARIANT(i <= j + 1 && i + j == n)
  CERASTES_DECREASES(j - i)
  for (i = 0, j = n; i < j; i++, j--) {
  }
  return i;
}
