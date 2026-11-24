// expect: error overflow @ L19
// mode: c23, c++20
// twin: tests/memory/forall_loop_arguments.c
// Pins: 0061 item 3 (audit round 1 B1): x + 1 may overflow, and the parameter m of the pure function is unused: the argument's obligation is a row of its own at the invariant, before the quantifier (one error: the entry and back-edge rows are the one line)
// Pre-registered from record 0061 item 3 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_none(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] >= 0;
}

int run(int x)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(ge_none, a, i, x + 1))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  (void)x;
  (void)a[0];
  return 0;
}
