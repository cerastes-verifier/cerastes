// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/pure_trusted_ensures.c
// Pins: 0062 item 4 (0061 item 3, M4.5e2): a pointer argument that a callee's atom names is, in a contract, a bare pointer parameter: &a[1] is unsupported_annotation
// Pre-registered from record 0062 (Part C), before the code: rejected. the tag is a question-160 candidate (mirrors forall_reject_base_address.c)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long tail_sum(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(&a[1], n - 1));

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
long long tail_sum_caller(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  return tail_sum(a, n);
}
