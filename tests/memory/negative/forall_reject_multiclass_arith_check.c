// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/forall_check_local.c
// Pins: 0061 question 261 (fail-closed narrowing, lifted by M4.5e2): in a check, a quantified E that reads two classes (a[k], a[k + 1]) and does arithmetic on the second class's read alone (a[k + 1] + 1) is unsupported_annotation at the forall (its row would read the second class's slot before that class's rows); contracts keep it (their rows are routed by ghost applications)
// Pre-registered from record 0061 questions 261 (audit round 3), before the code: unsupported_annotation at the forall
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool step(int k, const int *a, int m)
{
  return !(0 <= k && k < m - 1) || a[k] < a[k + 1] + 1;
}

int run(int c)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(cerastes_forall(step, a, 4));
  (void)a[0];
  (void)c;
  return 0;
}
