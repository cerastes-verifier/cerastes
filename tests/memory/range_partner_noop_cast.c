// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 3 (the count as VIR holds it): a written cast to the count's own type, (int)n for an int n, is dropped by the front end of VIR, so (int)n and n are one count and the partner is found, in either clause
// Pre-registered from record 0060 item 3, before the code: verifies
#include <cerastes.h>

int ens_cast(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, (int)n) && r == n)
{
  (void)p;
  return n;
}

int req_cast(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, (int)n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
{
  (void)p;
  return n;
}
