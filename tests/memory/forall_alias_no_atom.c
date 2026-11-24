// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 7 (audit round 2 issue 1): a quantified read through q, a pointer with no atom of its own that the contract equates with p (q == p), is framed by p's owns_range atom (an owning site frames the read; only a marked site does not: forall_marked_alias_pre_state.c): the f__wf of q has the type range of its field from its read, and the caller passes one array twice
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int first(int *p, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && q == p && cerastes_forall(nonneg, q, n))
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r >= 0)
{
  (void)q;
  (void)n;
  return p[0];
}

int use(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[2] = {1, 2};
  return first(a, a, 2);
}
