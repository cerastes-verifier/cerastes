// expect: error permission @ L22
// expect: error precondition @ L22
// expect: error overflow @ L22
// mode: c23, c++20
// twin: tests/memory/pure_check_parameters.c
// Pins: 0062 item 7 (S46 (f)): an application in a check over a parameter whose atom does not cover the callee's interval (a count one past the atom's): the framing row, permission, at the check
// Pre-registered from record 0062 (Part B), before the code: permission. Corrected under question 160 after the first run (build/m4.5e1d/tm2/run-b.log): three errors at the check, listed: precondition (the callee's pure line `offset_of(a) + n + 1 <= len(block_of(a))`, asserted first: question 278), permission (the framing row) and overflow (the argument `n + 1`, unbounded: the requires gives n no upper bound). Original note: (the first run decides the category of the bound row of a parameter's range: question 160)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int over_read(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  CERASTES_CHECK(sum_to(a, n + 1) == sum_to(a, n + 1));
  (void)a;
  return n;
}
