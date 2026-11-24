// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 1 (M4.5e2): an index a[n - 1 - i] is not i, i + c or i - c: unsupported_annotation
// Pre-registered from record 0061 item 1 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool reversed(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[n - 1 - i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(reversed, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
