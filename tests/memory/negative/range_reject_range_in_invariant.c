// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 1 (M4.5e2, question 237; audit round 2, S2): a range atom written in a loop invariant is rejected, naming M4.5e2 (the loop's attributes and its while are on one line: a diagnostic on the attribute reads the same line in both modes)
// Pre-registered from record 0060 items 1 and 3 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

int f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && n >= 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns_range(p, 1)) CERASTES_DECREASES(n - i) while (i < n) {
    i = i + 1;
  }
  return 0;
}
