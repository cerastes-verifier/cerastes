// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/memory/forall_must_init_filler.c
// Pins: 0061 item 9 (S45 (c) nminus1_fail): the loop fills n - 1 elements: the last element's flag is missing, so the must_init return row fails at the closing brace (one error)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

void fill(int *p CERASTES_MUST_INIT, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n - 1 && cerastes_forall(zero_at, p, i))
  CERASTES_DECREASES(n - 1 - i)
  while (i < n - 1) {
    p[i] = 0;
    i++;
  }
}
