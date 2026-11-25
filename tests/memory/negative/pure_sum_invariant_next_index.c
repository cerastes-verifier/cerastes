// expect: error permission @ L25
// expect: error invariant-entry @ L25
// expect: error invariant-preserved @ L25
// expect: error postcondition @ L31
// mode: c23, c++20
// twin: tests/memory/pure_sum.c
// Pins: 0062 item 7 (S46 (d), question 277 "Expect all"): the invariant applies sum_to one element past what the loop holds (i + 1): the framing assert fails at entry and at the back edge (one line, printed once), the invariant at entry and as preserved, and the return's postcondition rests on the invariant. The requires adds n < 2147483647 so that i + 1 has no overflow row of its own
// Pre-registered from record 0062 (Part D), before the code: permission, invariant-entry, invariant-preserved, postcondition. the count is a question-160 candidate (S46 (d) lists five Silicon errors, the two framing asserts sharing a C line; the harness's merging of the entry and back-edge framing rows is the open point)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long sum(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n))
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == sum_to(a, i + 1))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
