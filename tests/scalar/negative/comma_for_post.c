// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/comma_for.c
// Pins: 0020 item 2: for n == 1 the loop ends with i == 1, not n / 2
#include <cerastes.h>

int meet(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == n / 2)
{
  int i, j;
  CERASTES_INVARIANT(i <= j + 1 && i + j == n)
  CERASTES_DECREASES(j - i)
  for (i = 0, j = n; i < j; i++, j--) {
  }
  return i;
}
