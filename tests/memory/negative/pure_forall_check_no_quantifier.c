// expect: error overflow @ L19
// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/pure_forall_check_sum.c
// Pins: 0062 items 3, 8 and 9 (audit round 2 R2-S1): the same sum without the quantified clause in its requires: the add in its body may overflow (the overflow error, in the function) and the ensures bound does not follow (the postcondition error, at the return)
// Pre-registered from record 0062 (Part B), before the code: overflow, postcondition. from the auditor's probe c2b_noq.c: both errors at the function's return line
#include <cerastes.h>
#include <stdbool.h>

CERASTES_PURE bool small_at(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || (-1000 <= a[i] && a[i] <= 1000);
}
CERASTES_PURE int sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -1000 * n <= r && r <= 1000 * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}
int main(void)
{
  int a[3] = {1, 2, 3};
  CERASTES_CHECK(sum_to(a, 3) <= 3000);
  return a[0];
}
