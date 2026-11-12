// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 5: element reads p[0] and p[n - 1] in a requires and an ensures under owns_range(p, n) && n > 0: the range atom frames them (g && offset_of(p) <= offset_of(q) < offset_of(p) + n)
// Pre-registered from record 0060 item 5, before the code: verifies
#include <cerastes.h>

int first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0] > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

int last(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[n - 1] > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[n - 1])
{
  return p[n - 1];
}
