// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 5 (audit N4): p[0] framed by another parameter's range (p == q, q's range), and q[0] beside *p of one place (a cell read and an element read share a ghost through q_a == q_b ==> hv_a == hv_b)
// Pre-registered from record 0060 item 5, before the code: verifies
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
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p && q[0] == *p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == *p)
{
  (void)q;
  return *p;
}
