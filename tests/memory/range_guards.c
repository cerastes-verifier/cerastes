// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 items 3, 4 and 9 and 0039 item 7: the atom under the guards n == 0 || and p == NULL ||: every conjunct under g ==> forall, the marked return rows inside if (g), the same guard on both sides (the partner rule by path holds)
// Pre-registered from record 0060 items 3, 4 and 9, before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int first_or(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, n == 0 || cerastes_owns_range(p, n))
{
  if (n == 0) {
    return 0;
  }
  return p[0];
}

int first_or_null(int *p, int n)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, p == NULL || cerastes_owns_range(p, n))
{
  if (p == NULL || n <= 0) {
    return 0;
  }
  return p[0];
}
