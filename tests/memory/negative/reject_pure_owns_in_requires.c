// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 items 1 and 3: owns_range in a pure function's requires is rejected (M4.5f1), at the atom
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int first(int *a, int n)
    CERASTES_REQUIRES(0 < n && cerastes_owns_range(a, n))
{
  return a[0];
}

int use(void)
{
  int a[2] = {1, 2};
  CERASTES_CHECK(first(a, 2) == first(a, 2));
  return a[0];
}
