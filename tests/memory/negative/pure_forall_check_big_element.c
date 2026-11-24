// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/memory/pure_forall_check_sum.c
// Pins: 0062 item 7 (cut 2; audit round 2 R2-S1): an element outside the quantified clause (5000): the check's application is the one precondition error (the Q row), at the check
// Pre-registered from record 0062 (Part B), before the code: precondition. from the auditor's probe c2c_big.c
#include <cerastes.h>
#include <stdbool.h>

CERASTES_PURE bool small_at(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || (-1000 <= a[i] && a[i] <= 1000);
}
CERASTES_PURE int sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n) && cerastes_forall(small_at, a, n))
    CERASTES_ENSURES(r, -1000 * n <= r && r <= 1000 * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}
int main(void)
{
  int a[3] = {1, 2, 5000};
  CERASTES_CHECK(sum_to(a, 3) <= 3000);
  return a[0];
}
