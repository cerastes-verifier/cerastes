// expect: error check @ L19
// mode: c23, c++20
// twin: tests/memory/pure_check_unfold_steps.c
// Pins: 0062 item 8 (S46 (d) recorded): the pinned false alarm: without the intermediate check, check(sum_to(a, 2) == a[0] + a[1]) is not proved (Silicon unfolds the function one step at a time); M4.5f3's lemmas lift it
// Pre-registered from record 0062 (Part D), before the code: check. a pinned false alarm, fail closed: if the pinned Silicon ever proves it, the test says so by failing
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
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(sum_to(a, 2) == a[0] + a[1]);
  return a[0];
}
