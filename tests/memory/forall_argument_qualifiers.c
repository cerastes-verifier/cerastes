// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 3: an `int *` argument for the `const int *` parameter of f is allowed (qualifiers may be added); the read through an owns_range place under the binder and a caller on a local array
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int first_of(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0 && cerastes_forall(nonneg, a, n))
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && r >= 0)
{
  (void)n;
  return a[0];
}

int caller(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[2] = {4, 5};
  return first_of(a, 2);
}
