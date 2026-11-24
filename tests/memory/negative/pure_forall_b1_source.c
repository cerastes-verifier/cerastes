// expect: error precondition @ L24
// mode: c23, c++20
// twin: tests/memory/pure_forall_check_sum.c
// Pins: 0062 item 8 and audit round 1 B1, at source level (audit round 2 R2-S1): g's ensures m < INT_MAX holds only because its quantified requires makes every element exceed m; h applies g on the m == INT_MAX arm without that clause, so the application is the precondition error (the Q row) and g's ensures is not assumed for it: h's m + 1 is not excused by it
// Pre-registered from record 0062 (Part B), before the code: precondition. from the auditor's probe c2d_b1_source.c; the line is the application in h's body
#include <cerastes.h>
#include <stdbool.h>

CERASTES_PURE bool gt(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || a[i] > m;
}

CERASTES_PURE int g(const int *a, int n, int m)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n) && cerastes_forall(gt, a, n, m))
    CERASTES_ENSURES(r, r == 0 && m < 2147483647)
{
  return 0 * n;
}

CERASTES_PURE int h(const int *a, int n, int m)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
{
  return (m == 2147483647 ? g(a, n, m) : 0) + (m + 1 > m);
}

int main(void)
{
  int a[1] = {5};
  CERASTES_CHECK(h(a, 1, 3) == 1);
  return a[0];
}
