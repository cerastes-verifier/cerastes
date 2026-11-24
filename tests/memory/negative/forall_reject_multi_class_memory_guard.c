// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 6 (owner decision, accepted fail-closed narrowing; lifted by M4.5e2): a multi-class quantifier (a[i] and b[i]) whose guard reads memory (b[0] > 0) is unsupported_annotation
// Pre-registered from record 0061 item 1 (Part B, owner decision after Part B's probes), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool guarded_same(int i, const int *a, const int *b, int n)
{
  return !(0 <= i && i < n && b[0] > 0) || a[i] == b[i];
}

void use(const int *a, const int *b, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_reads_range(b, n) && n > 0 && cerastes_forall(guarded_same, a, b, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n) && cerastes_reads_range(b, n))
{
  (void)a;
  (void)b;
  (void)n;
}
