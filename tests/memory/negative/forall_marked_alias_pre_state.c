// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/forall_must_init_ensures.c
// Pins: 0061 item 7 (marked sites frame no quantified read; 0042 item 6's alias rule; audit round 1 S7): q aliases the ref_to_uninit parameter p, whose site never frames a pre-state read: the quantified read through q is unowned (permission in the f__wf row, reported at the clause)
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void alias(int *p CERASTES_REF_TO_UNINIT, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && q == p && cerastes_forall(nonneg, q, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)q;
  (void)n;
}
