// expect: error check @ L36
// mode: c23, c++20
// twin: tests/memory/pure_sum_caller.c
// Pins: 0062 item 7 (S46 (e)): the caller writes a[1] between the call and the check, so sum_to(a, 2) is another value and the check fails (one error, at the fact). The two-element twin of tests/long/memory/negative/pure_sum_caller_write_between.c (four elements: 237-356 s on the x86-64 box, moved to the long tier, owner question 285): the default tier keeps a must-fail twin of pure_sum_caller.c (a long twin never stands in place of the ordinary one, owner question 207)
// Pre-registered in M4.5e1d (task 5), before its first run; the cost of a refutation grows with the element count (2 elements about 34 s on the box, il-analyst-3 report 6)
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
  int a[2] = {1, 2};
  long long x = sum(a, 2);
  a[1] = 7;
  CERASTES_CHECK(x == sum_to(a, 2));
  (void)x;
  return a[0];
}
