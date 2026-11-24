// expect: error precondition @ L18
// mode: c23, c++20
// twin: tests/memory/pure_contract_callee_requires_given.c
// Pins: 0062 item 7 (S46 (b)): the contract does not give the callee's n <= 1000: one precondition error at the ensures, with a note at the callee's clause; Viper's own application.precondition is silent
// Pre-registered from record 0062 (Part C), before the code: precondition
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long total(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n));

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long total_caller(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  return total(a, n);
}
