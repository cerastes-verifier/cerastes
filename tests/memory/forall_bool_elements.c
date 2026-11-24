// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 7 (a), audit round 1 S3: bool elements under a binder in a contract (a Bool ghost function with no type range): a store that sets every flag proves the quantified ensures, a requires over the flags gives the first one
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool all_true(int i, const bool *a, int n)
{
  return !(0 <= i && i < n) || a[i];
}

void set_true2(bool *p)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && cerastes_forall(all_true, p, 2))
{
  p[0] = true;
  p[1] = true;
}

bool first_flag(const bool *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0 && cerastes_forall(all_true, a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r)
{
  (void)n;
  return a[0];
}
