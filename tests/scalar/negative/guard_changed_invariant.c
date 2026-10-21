// expect: error division-by-zero @ L12
// mode: c23, c++20
// twin: tests/scalar/guard_kept.c
// Pins: 0010 item 11: the guard `i == 0 ||` of a loop invariant changed to `i == 1 ||`; 100 / i is evaluated at entry
#include <cerastes.h>

int by_invariant(int n)
    CERASTES_REQUIRES(n >= 1 && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 1 || 100 / i >= 1))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
