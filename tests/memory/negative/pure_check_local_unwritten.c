// expect: error uninitialized @ L21
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 item 7 (S46 (f)): one unwritten element under an application in a check: the flag row, uninitialized, at the check
// Pre-registered from record 0062 (Part B), before the code: uninitialized
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
  int a[3];
  a[0] = 1;
  a[1] = 2;
  CERASTES_CHECK(sum_to(a, 3) == sum_to(a, 3));
  return a[0];
}
