// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/range_framing_alias.c
// Pins: 0060 item 5 (audit N4): without q == p the cell atom of *p does not frame q[0]: permission at the clause (cerastes_live(q) keeps q a usable pointer, so the body's (void)q; is no second reason)
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

int via(int *p, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns_range(q, n) && n > 0 && p == q && p[0] > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(q, n) && r == q[0])
{
  (void)p;
  (void)n;
  return q[0];
}

int both(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_live(q) && q[0] == *p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == *p)
{
  (void)q;
  return *p;
}
