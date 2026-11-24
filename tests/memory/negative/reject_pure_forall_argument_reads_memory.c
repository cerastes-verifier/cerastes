// expect: rejected cerastes-unsupported_annotation @ L23
// mode: c23, c++20
// twin: tests/memory/pure_forall_check_sum.c
// Pins: 0062 item 7 (audit round 2 R2-S1; 0061 item 3, M4.5e2): an application that passes an argument that reads memory (a[0] - 1) to a parameter that a quantified clause of the callee's requires names is rejected
// Pre-registered from record 0062 (Part B), before the code: rejected. from the auditor's probe c2e_forall_arg_reads.c
#include <cerastes.h>
#include <stdbool.h>

CERASTES_PURE bool gt(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || a[i] > m;
}

CERASTES_PURE int g(const int *a, int n, int m)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n) && cerastes_forall(gt, a, n, m))
{
  return n + 0 * m;
}

int main(void)
{
  int a[2] = {5, 6};
  CERASTES_CHECK(g(a, 2, a[0] - 1) == 2);
  return a[0];
}
