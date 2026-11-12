// expect: error out-of-bounds @ L13
// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/range_empty.c
// Pins: 0060 item 4 (S43 N1): p[0] under an empty range: the bound row (the object may end at p) and the slot's permission each fail at the read
// Pre-registered from record 0060 item 4, before the code
#include <cerastes.h>

int none(int *p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 0))
    CERASTES_ENSURES(r, cerastes_owns_range(p, 0))
{
  return p[0];
}

int zero(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
{
  (void)p;
  return n;
}
