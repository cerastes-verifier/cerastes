// expect: error precondition @ L19
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 item 7 (S46 (f), question 278): a count past a local array in an application in a check: the callee's pure lines are asserted before the bound and marked rows, so the callee's requires `offset_of(a) + n <= len(block_of(a))` is the ONE error, precondition, at the check (not out-of-bounds)
// Pre-registered from record 0062 (Part B), before the code: precondition (re-pointed from out-of-bounds by the owner's answer to question 278, before any run)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int use(void)
{
  int a[3] = {1, 2, 3};
  CERASTES_CHECK(sum_to(a, 4) == sum_to(a, 4));
  return a[0];
}
