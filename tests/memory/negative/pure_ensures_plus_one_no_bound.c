// expect: error overflow @ L13
// expect: error overflow @ L18
// mode: c23, c++20
// twin: tests/memory/pure_ensures_plus_one.c
// Pins: 0062 items 8 and 9 (S46 (c) (1), (2)): sum_to without its ensures overflows in its own body (the function's definition check, at the operation) and, with no fact (ii), the contract's + 1 overflows too: both listed
// Pre-registered from record 0062 (Part C), before the code: overflow, overflow. the count is a question-160 candidate (S46 measured each row alone: one error each)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long plus_one(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n) + 1);

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long plus_one_caller(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  return plus_one(a, n);
}
