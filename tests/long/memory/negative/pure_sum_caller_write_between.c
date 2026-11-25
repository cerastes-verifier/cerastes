// expect: error check @ L36
// mode: c23, c++20
// twin: tests/memory/pure_sum_caller.c
// Pins: 0062 item 7 (S46 (e)): the caller writes a[1] between the call and the check: sum_to(a, 4) is now another value, so the check fails (one error, at the fact). Moved to tests/long/ in M4.5e1d (owner question 285: 226-356 s measured on the x86-64 box). Pre-registered from record 0062 (Part D), before the code: check
// timeout: 1800
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
  a[1] = 7;
  CERASTES_CHECK(x == sum_to(a, 4));
  (void)x;
  return a[0];
}
