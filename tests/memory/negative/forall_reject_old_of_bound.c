// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/forall_set_one_old.c
// Pins: 0061 item 8 (audit round 2 issue 7): cerastes_old of the bound variable is rejected with the generic message: unsupported_annotation at the quantifier
// Pre-registered from record 0061 item 8 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool g(int i, const int *a, int n)
{
  return !(0 <= cerastes_old(i) && i < n) || a[i] >= 0;
}

void f(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n))
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_forall(g, a, n))
{
  (void)a;
  (void)n;
}
