// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_in_loop.c
// Pins: 0019 items 4 and 6: both cases assign and read; the total is wrong
#include <cerastes.h>

int run(int n, int code)
    CERASTES_REQUIRES(0 <= n && n <= 100 && 0 <= code && code <= 1)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int total = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    switch (code) {
      int x;
    case 0:
      x = 2;
      total += x;
      break;
    default:
      x = 3;
      total += x;
      break;
    }
    i += 1;
  }
  return total;
}
