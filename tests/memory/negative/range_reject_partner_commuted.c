// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/range_partners.c
// Pins: 0060 item 3 (the structural equality, the advisor's point): n + 1 against 1 + n are structurally different counts, so the ensures atom has no partner and is rejected; the record says counts are compared as written, not by value
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_REQUIRES(cerastes_owns_range(p, n + 1))
    CERASTES_ENSURES(r, cerastes_owns_range(p, 1 + n) && r == n)
{
  (void)p;
  return n;
}
