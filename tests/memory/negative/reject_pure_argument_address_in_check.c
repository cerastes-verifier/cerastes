// expect: rejected cerastes-unsupported_annotation @ L19
// mode: c23, c++20
// twin: tests/memory/pure_check_local_array.c
// Pins: 0062 item 4 (0061 item 3, M4.5e2): a pointer argument that a callee's atom names is, in a check, a pointer variable that is not address-taken or a named local array: &a[1] is unsupported_annotation
// Pre-registered from record 0062 (Part B), before the code: rejected. the tag and the line are question-160 candidates (mirrors forall_reject_base_address.c)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int use(void)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(sum_to(&a[1], 2) == sum_to(&a[1], 2));
  return a[0];
}
