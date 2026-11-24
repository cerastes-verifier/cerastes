// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 6 and 7 (S46 (f)): a pure function over reads_range applied in a check on a local array: the rows of 0061 item 10 over its interval, then the precondition rows, then the fact
// Pre-registered from record 0062 (Part B), before the code: verifies. Edited before any run (audit round 2, R2-S3): the check observes the value, so a wrong slot or swapped arguments fails; its off-by-one twin is the negative of the same name with _value_off_by_one
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
  CERASTES_CHECK(sum_to(a, 3) == sum_to(a, 3));
  CERASTES_CHECK(sum_to(a, 1) == 1);
  return a[0];
}
