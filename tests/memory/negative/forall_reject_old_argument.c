// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 (audit round 1 B1; lifted by M4.5e2): cerastes_old as a trailing argument is unsupported_annotation with the memory-reading arguments
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_m(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || a[i] >= m;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n) && n > 0 && cerastes_forall(ge_m, a, n, cerastes_old(a[0])))
{
  (void)a;
  (void)n;
}
