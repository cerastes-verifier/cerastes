// expect: rejected cerastes-unsupported_expression @ L13
// expect: rejected cerastes-unsupported_expression @ L17
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 4 (audit round 2 issue 2; 0041 item 12): a global inside E is unsupported_expression, at the pure function's own body (its 0041 check) and at the quantifier: two errors
// Pre-registered from record 0061 item 4 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

int g_global;
CERASTES_PURE bool g(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= g_global;
}

void f(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(g, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
