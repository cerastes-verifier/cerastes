// expect: error permission @ L23
// expect: error invariant-preserved @ L23
// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/memory/pure_sum.c
// Pins: 0062 item 7 (S46 (d) unframed, question 277): the invariant applies sum_to over b, of which no atom gives the loop anything: the framing assert (entry and back edge: one line, printed once) is a permission error, and the application defaults to 0 while s grows, so the invariant is not preserved; and (question 283, "rows, as elsewhere") the callee's pure line `b != null` is a precondition row at the invariant. Pre-registered from record 0062 (Part D), before the code: permission, invariant-preserved; precondition added before the first harness run, from il-analyst-3 report 3 (the driver prints all three, both modes, at L23)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long unframed(const int *a, const int *b, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == sum_to(b, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
