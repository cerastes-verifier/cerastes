// expect: error invariant-preserved @ L18
// mode: c23, c++20
// twin: tests/scalar/uninit_switch_decl_loop_in_range.c
// Pins: 0019 items 4 and 5: the loop assigns x before reading it; the value is wrong, so the invariant is not preserved
#include <cerastes.h>

int run(int code, int n)
    CERASTES_REQUIRES(0 <= code && code <= 1 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, (code == 0 && r == 0) || (code == 1 && r == n))
{
  int total = 0;
  switch (code) {
    int x;
  case 0:
    break;
  case 1: {
    int i = 0;
    CERASTES_INVARIANT(0 <= i && i <= n && total == i)
    CERASTES_DECREASES(n - i)
    while (i < n) {
      x = i + 2;
      total = x;
      i += 1;
    }
    break;
  }
  }
  return total;
}
