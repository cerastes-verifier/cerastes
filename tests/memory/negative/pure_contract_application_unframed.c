// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/pure_trusted_ensures.c
// Pins: 0062 item 7 (S46 (b), S45's twin): the contract applies sum_to over one element more than its atom owns: the framing row is the one error (permission, at the ensures); the rest of the application's rows hold after it and Viper's own check is silent
// Pre-registered from record 0062 (Part C), before the code: permission
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long over(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n < 1000 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n + 1));

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long over_caller(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n < 1000 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  return over(a, n);
}
