// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 items 4 and 10: p[i] and p[i] = 0 under owns_range(p, n) with 0 <= i < n: the bound row follows from the atom's pure line (offset_of(p) + n <= len) and the index's range, the slot's permission from the quantifier
// Pre-registered from record 0060 items 4 and 10, before the code: verifies
#include <cerastes.h>

int first(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  (void)n;
  return p[i];
}

void clear(int *p, int n, int i)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)n;
  p[i] = 0;
}
