// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/range_partner_noop_cast.c
// Pins: 0060 item 3: ens_cast claims r == 0 where it returns n: the postcondition fails at the return (the partners themselves are found)
// Pre-registered from record 0060 item 3, before the code
#include <cerastes.h>

int ens_cast(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, (int)n) && r == 0)
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
