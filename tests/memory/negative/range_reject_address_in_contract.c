// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/memory/range_get_set.c
// Pins: 0060 item 1 (M4.5e2, question 236; audit round 2, S2): &p[e] in a contract is rejected, with a message that names M4.5e2, and with exactly one diagnostic (the partner of the ensures atom is still found)
// Pre-registered from record 0060 items 1 and 3 (audit round 2), before the code
#include <cerastes.h>
#include <stddef.h>

int f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1 && &p[1] != p)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
  return 0;
}
