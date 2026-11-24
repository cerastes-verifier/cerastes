// expect: error precondition @ L26
// mode: c23, c++20
// twin: tests/memory/forall_argument_qualifiers.c
// Pins: 0061 item 3 (twin): a negative element: the precondition row fails at the call
// Pre-registered from record 0061 item 7 (Part B), before the code
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
  int a[2] = {4, -5};
  return first_of(a, 2);
}
