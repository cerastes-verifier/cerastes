// expect: error invariant-preserved @ L22
// mode: c23, c++20
// twin: tests/memory/pure_sum_copy.c
// Pins: 0062 item 7 (S46 (d) copy): the loop of pure_sum_copy.c with the invariant over the array it fills (b, overwritten with 0 while s adds a[i]): the write is inside what sum_to(b, i + 1) reads, so the invariant is not preserved (invariant-preserved at the clause)
// Pre-registered from record 0062 (Part D), before the code: invariant-preserved. the count is a question-160 candidate (the ensures claims no value, so only the invariant row can fail)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long sum_copy(const int *a, int *b, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n) && cerastes_owns_range(b, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && cerastes_owns_range(b, n))
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == sum_to(b, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    b[i] = 0;
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
