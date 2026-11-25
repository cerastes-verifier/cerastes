// expect: error invariant-preserved @ L21
// mode: c23, c++20
// twin: tests/memory/pure_sum_local.c
// Pins: 0062 item 7: the same loop over a local array writes a[0], inside the prefix the invariant sums: the function's value changed, so the invariant is not preserved (invariant-preserved at the clause)
// Pre-registered from record 0062 (Part D), before the code: invariant-preserved
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int main(void)
{
  int a[4] = {1, 2, 3, 4};
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && s == sum_to(a, i))
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    a[0] = 0;
    s = s + a[i];
    i = i + 1;
  }
  CERASTES_CHECK(s == sum_to(a, 4));
  CERASTES_CHECK(s <= 4 * 2147483647LL);
  return 0;
}
