// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 5: cerastes_old(p[j]) under owns_range: a store to p[i] with j != i keeps p[j] (the C spelling: address_of(subscript), never folded)
// Pre-registered from record 0060 item 5, before the code: verifies
#include <cerastes.h>

void set(int *p, int n, int i, int j, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n && 0 <= j && j < n && j != i)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i] == v && p[j] == cerastes_old(p[j]))
{
  (void)n;
  (void)j;
  p[i] = v;
}
