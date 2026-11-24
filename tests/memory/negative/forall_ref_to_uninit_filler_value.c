// expect: error invariant-preserved @ L19
// mode: c23, c++20
// twin: tests/memory/forall_ref_to_uninit_filler.c
// Pins: 0061 items 4 and 9 (twin): the filler stores 1: the user clause is not preserved (one error)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

void fill(int *p CERASTES_REF_TO_UNINIT, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(zero_at, p, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i] = 1;
    i++;
  }
}
