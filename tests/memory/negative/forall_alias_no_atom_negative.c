// expect: error precondition @ L27
// mode: c23, c++20
// twin: tests/memory/forall_alias_no_atom.c
// Pins: 0061 item 7 (twin): the caller's array holds a negative element: the precondition row of the quantified clause fails at the call
// Pre-registered from record 0061 item 7 (Part B), before the code
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
  int a[2] = {1, -2};
  return first(a, a, 2);
}
