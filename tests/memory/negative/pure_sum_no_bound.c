// expect: error overflow @ L13
// expect: error overflow @ L25
// mode: c23, c++20
// twin: tests/memory/pure_sum.c
// Pins: 0062 items 8 and 9 (S46 (d) noens_wf): sum_to without its bound ensures: the function's definition check reports the add in its body (overflow, in the function, at the operation) and the loop's s + a[i] overflows too, as nothing bounds s: both expected
// Pre-registered from record 0062 (Part D), before the code: overflow, overflow. the count is a question-160 candidate (the table in the plan lists both)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
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
