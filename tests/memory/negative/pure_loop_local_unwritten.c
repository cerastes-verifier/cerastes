// expect: error uninitialized @ L23
// mode: c23, c++20
// twin: tests/memory/pure_loop_local_bound.c
// Pins: 0062 item 7 (S46 (d)): a[2] is unwritten under the invariant's application: the flag row over the interval is the one error (uninitialized, at the clause); the loop reads only a[0] and a[1]
// Pre-registered from record 0062 (Part D), before the code: uninitialized. the count is a question-160 candidate (the entry flag row alone: the value clause holds by the function's ensures)
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
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(sum_to(a, 3) <= 6442450941LL)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return a[0];
}
