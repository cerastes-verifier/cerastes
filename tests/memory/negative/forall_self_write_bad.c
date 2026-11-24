// expect: error invariant-preserved @ L19
// mode: c23, c++20
// twin: tests/memory/forall_self_write.c
// Pins: 0061 item 9 (twin): the loop stores a[i] + 1: the quantified clause is not preserved (one error: r == 3 follows from the assumed invariant)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool kept_at(int k, const int *a, int m)
{
  return !(0 <= k && k < m) || a[k] == k + 1;
}

int f(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(kept_at, a, 3))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a[i] = a[i] + 1;
    i = i + 1;
  }
  return a[2];
}
