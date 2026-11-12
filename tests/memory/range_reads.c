// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 items 3, 4, 5 and 9: reads_range(p, n) with a ghost amount: reads of p[i] in requires and ensures, a post value equal to the pre value (cerastes_old of an element), the borrowed range handed back
// Pre-registered from record 0060 items 3, 4, 5 and 9, before the code: verifies
#include <cerastes.h>

int peek(const int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == p[i])
{
  (void)n;
  return p[i];
}

int same(const int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && p[i] == cerastes_old(p[i]) && r == p[i])
{
  (void)n;
  return p[i];
}
