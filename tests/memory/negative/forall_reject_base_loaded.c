// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 1 (M4.5e2): a quantified read through a loaded pointer *pp is unsupported_annotation (the argument reads memory)
// Pre-registered from record 0061 item 1 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_forall(nonneg, *pp, n))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  (void)pp;
  (void)n;
}
