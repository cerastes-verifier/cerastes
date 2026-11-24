// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 7 (a), audit round 1 S3: pointer elements under a binder in a contract (a Ref ghost function whose range is well-formedness): storing NULL in both slots proves the quantified ensures
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool all_null(int i, int **a, int n)
{
  return !(0 <= i && i < n) || a[i] == NULL;
}

void clear2(int **p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && cerastes_forall(all_null, p, 2))
{
  p[0] = NULL;
  p[1] = NULL;
}
