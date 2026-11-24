// expect: error invariant-preserved @ L19
// mode: c23, c++20
// twin: tests/memory/forall_fill_range.c
// Pins: 0061 item 9 (twin): the loop stores 1: the user clause of the invariant is not preserved (one error: the ensures follows from the assumed invariant)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

void zero(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_forall(zero_at, p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(zero_at, p, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i] = 1;
    i++;
  }
}
