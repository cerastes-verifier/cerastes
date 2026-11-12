// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 4: an empty range (a literal 0, and n == 0) holds the liveness fraction only: nothing to read, nothing to give back but the atom
// Pre-registered from record 0060 item 4, before the code: verifies
#include <cerastes.h>

int none(int *p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 0))
    CERASTES_ENSURES(r, cerastes_owns_range(p, 0))
{
  (void)p;
  return 0;
}

int zero(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n == 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
{
  (void)p;
  return n;
}
