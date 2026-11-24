// expect: error precondition @ L19
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 item 7 (S46 (f)): a negative count in an application in a check: the callee's value requires `0 <= n` is the precondition row at the check
// Pre-registered from record 0062 (Part B), before the code: precondition (question 278: the pure lines come first, so the negative count is the one precondition error; open to a question-160 correction after the first run)
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
  CERASTES_CHECK(sum_to(a, -1) == sum_to(a, -1));
  return a[0];
}
