// expect: error invariant-preserved @ L12
// mode: c23, c++20
// twin: tests/scalar/comma_for.c
// Pins: 0020 item 2: i <= j is not preserved when j == i + 1
#include <cerastes.h>

int meet(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == (n + 1) / 2)
{
  int i, j;
  CERASTES_INVARIANT(i <= j && i + j == n)
  CERASTES_DECREASES(j - i)
  for (i = 0, j = n; i < j; i++, j--) {
  }
  return i;
}
