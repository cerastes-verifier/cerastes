// expect: error precondition @ L28
// mode: c23, c++20
// twin: tests/memory/forall_guards.c
// Pins: 0061 item 7 (S45 (a) twin): the guard n == 0 is false for a two-element array, so the quantifier is asked, and the negative element fails the precondition row
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int by_guard(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && (n == 0 || cerastes_forall(nonneg, a, n)))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r >= 0)
{
  if (n == 0) {
    return 0;
  }
  return a[0];
}

int use(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[2] = {1, -2};
  return by_guard(a, 2);
}
