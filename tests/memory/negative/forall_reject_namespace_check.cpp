// expect: rejected cerastes-unsupported_annotation @ L19
// expect: frontend-warning set_but_not_used @ L15
// expect: frontend-warning set_but_not_used @ L15
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 2 (cut 3): in a check (today the call is call_in_annotation there, and no test pins it)
// Pre-registered from record 0061 item 2 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  CERASTES_CHECK(cerastes::forall(nonneg, a, n));
  (void)a;
  (void)n;
}
