// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/scalar/for_loop.c
// Pins: 0012 item 8: total += 3 breaks total == 2 * i
#include <cerastes.h>

int twice_count(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == 2 * i)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++) {
    total += 3;
  }
  return total;
}
