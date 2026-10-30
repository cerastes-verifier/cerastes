// expect: error pointer-comparison @ L14
// mode: c23, c++20
// twin: tests/memory/loop_condition_compare.c
// Pins: 0038 item 2: with live(p) and live(q) alone either may point one past the end of an object, so the comparison in the condition is rejected (0037 item 12)
#include <cerastes.h>

int walk(int *p, int *q, int n)
    CERASTES_REQUIRES(cerastes_live(p) && cerastes_live(q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_live(p) && cerastes_live(q))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n && p != q) {
    i += 1;
  }
  return i;
}
