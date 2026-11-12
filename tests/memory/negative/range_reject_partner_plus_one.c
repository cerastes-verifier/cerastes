// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/range_partners.c
// Pins: 0060 items 1 and 3 (M4.5e2): an ensures range with another count than its requires partner (n against n + 1) is rejected at the ensures atom
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n + 1) && r == n)
{
  (void)p;
  return n;
}
