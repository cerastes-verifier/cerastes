// expect: error permission @ L9
// mode: c23, c++20
// twin: tests/memory/range_framing_alias.c
// Pins: 0060 item 5 (audit N4): without p == q the range of q does not frame p[0]: permission at the clause
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

int via(int *p, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns_range(q, n) && n > 0 && p[0] > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(q, n) && r == q[0])
{
  (void)p;
  (void)n;
  return q[0];
}

int both(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p && q[0] == *p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == *p)
{
  (void)q;
  return *p;
}
