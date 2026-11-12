// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/range_partners.c
// Pins: 0060 item 3: a written cast that changes the count's value or type ((unsigned)n against n) keeps its meaning on VIR: the counts differ, the ensures atom has no partner and is rejected at the ensures atom (a same-type cast such as (int)n is dropped by the front end of VIR and is the same count: range_partner_noop_cast.c)
// Pre-registered from record 0060 items 1 and 3, before the code
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, (unsigned)n) && r == n)
{
  (void)p;
  return n;
}
