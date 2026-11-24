// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 6 (owner decision, accepted fail-closed narrowing; lifted by M4.5e2): arithmetic combining two classes' reads (a[i] + b[i] < 100) is unsupported_annotation; a comparison of two classes (a[i] == b[i], a[i] <= a[i + 1]) is accepted: forall_two_arrays.c, forall_neighbours.c
// Pre-registered from record 0061 item 1 (Part B, owner decision after Part B's probes), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool sum_below(int i, const int *a, const int *b, int n)
{
  return !(0 <= i && i < n) || a[i] + b[i] < 100;
}

void use(const int *a, const int *b, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_reads_range(b, n) && n > 0 && cerastes_forall(sum_below, a, b, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n) && cerastes_reads_range(b, n))
{
  (void)a;
  (void)b;
  (void)n;
}
