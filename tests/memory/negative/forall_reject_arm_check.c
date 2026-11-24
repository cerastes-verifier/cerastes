// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/forall_check_local.c
// Pins: 0061 question 262 (fail-closed narrowing, lifted by M4.8 with question 249's positions): in a check, a forall as an arm of ?: is unsupported_annotation at the forall; contracts accept both (forall_guards.c); only a top-level conjunct is split (forall_check_conjunct.c)
// Pre-registered from record 0061 questions 262 (audit round 3), before the code: unsupported_annotation at the forall
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int k, const int *a, int n)
{
  return !(0 <= k && k < n) || a[k] >= 0;
}

int run(int c)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(c > 0 ? cerastes_forall(nonneg, a, 4) : 1);
  (void)a[0];
  (void)c;
  return 0;
}
