// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/memory/range_loop_invariant_element.c
// Pins: 0060 item 11 (A2's twin): the body stores -1 to *p: p[0] >= 0 is not preserved: invariant-preserved at the invariant clause
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>

int count(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10 && *p >= 0)
{
  (void)p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0] >= 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = -1;
    i += 1;
  }
  return i;
}
