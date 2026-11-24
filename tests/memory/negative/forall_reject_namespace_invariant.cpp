// expect: rejected cerastes-unsupported_annotation @ L19
// expect: frontend-warning set_but_not_used @ L14
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 2 (cut 3): in a loop invariant
// Pre-registered from record 0061 item 2 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n >= 0)
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes::forall(nonneg, a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  (void)a;
}
