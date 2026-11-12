// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 items 3, 4, 5 and 9: a getter and a setter under owns_range(p, n) with 0 <= i < n: the element reads of the ensures are framed by the atom, the accesses pass the bound and permission rows, the return rows give the range back
// Pre-registered from record 0060 items 3, 4, 5 and 9, before the code: verifies
#include <cerastes.h>

int get(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[i])
{
  (void)n;
  return p[i];
}

void set(int *p, int n, int i, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i] == v)
{
  (void)n;
  p[i] = v;
}
