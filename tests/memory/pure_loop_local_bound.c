// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 7 (S46 (d) local): a loop whose invariant applies sum_to(a, 3) over a local array: the loop carries the array's place and the flags of the interval hold on entry (every element written), and the bound is the function's ensures
// Pre-registered from record 0062 (Part D), before the code: verifies
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int local_loop(void)
{
  int a[3];
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(sum_to(a, 3) <= 6442450941LL)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return a[0];
}
