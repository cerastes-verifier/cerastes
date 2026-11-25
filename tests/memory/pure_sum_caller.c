// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 7 (S46 (e)): a caller of sum on a local array: x = sum(a, 4) takes its ensures from the postcondition r == sum_to(a, n), read in the caller's state, so check(x == sum_to(a, 4)) holds
// Pre-registered from record 0062 (Part D), before the code: verifies
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long sum(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n))
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == sum_to(a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}

int use(void)
{
  int a[4] = {1, 2, 3, 4};
  long long x = sum(a, 4);
  CERASTES_CHECK(x == sum_to(a, 4));
  (void)x;
  return a[0];
}
