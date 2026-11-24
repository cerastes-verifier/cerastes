// expect: rejected cerastes-unsupported_annotation @ L14
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 2 (cut 3): in verify mode every cerastes::forall is unsupported_annotation (write cerastes_forall: M7), in a requires
// Pre-registered from record 0061 item 2 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes::forall(nonneg, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
