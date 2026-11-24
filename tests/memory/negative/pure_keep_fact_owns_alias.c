// expect: error overflow @ L18
// mode: c23, c++20
// twin: tests/memory/pure_keep_fact.c
// Pins: 0062 item 8 fact (iii) (S46 (c) (4)): the application is over an owns_range alias of a guarded reads_range: the keep fact holds under both guards only, so the post value is not tied to the pre value and the added constant may overflow
// Pre-registered from record 0062 (Part C), before the code: overflow. the tag is a question-160 candidate (S46 (c) (4) fails on the overflow row alone)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long keep_alias(int *a, int *b, int c, int n)
    CERASTES_REQUIRES(b == a && 0 <= n && (c == 0 || cerastes_reads_range(a, n)) && cerastes_owns_range(b, n) && sum_to(b, n) < 100)
    CERASTES_ENSURES(r, (c == 0 || cerastes_reads_range(a, n)) && cerastes_owns_range(b, n) && r == sum_to(b, n) + 9223372036854775000LL);

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long keep_alias_caller(int *a, int *b, int c, int n)
    CERASTES_REQUIRES(b == a && 0 <= n && (c == 0 || cerastes_reads_range(a, n)) && cerastes_owns_range(b, n) && sum_to(b, n) < 100)
    CERASTES_ENSURES(r, (c == 0 || cerastes_reads_range(a, n)) && cerastes_owns_range(b, n))
{
  return keep_alias(a, b, c, n);
}
