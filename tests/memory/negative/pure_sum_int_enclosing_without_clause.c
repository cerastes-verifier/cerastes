// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/memory/pure_sum_int.c
// Pins: 0062 items 7 and 8 (S46 (h) enclosing): a contract that applies sum_to and does not give its quantified clause: one precondition error at the ensures (the Q row), no native error
// Pre-registered from record 0062 (Part D), before the code: precondition
#include <cerastes.h>

CERASTES_PURE bool small_at(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || (-1000 <= a[i] && a[i] <= 1000);
}

CERASTES_PURE int sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n) && cerastes_forall(small_at, a, n))
    CERASTES_ENSURES(r, -1000 * n <= r && r <= 1000 * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int total(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n));

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
int total_caller(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  return total(a, n);
}
