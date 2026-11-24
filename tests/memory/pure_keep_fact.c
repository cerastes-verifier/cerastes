// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 8 fact (iii) (S46 (c) (3) adapted, question 276): a reads_range atom in the requires and its partner in the ensures: the post-state application equals the pre-state one, so the sum stays below 100 and the added constant does not overflow
// Pre-registered from record 0062 (Part C), before the code: verifies
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long keep(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n) && sum_to(a, n) < 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n) + 9223372036854775000LL);

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long keep_caller(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n) && sum_to(a, n) < 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  return keep(a, n);
}
