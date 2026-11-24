// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 question 259 (fail-closed narrowing, lifted by M4.5e2): a read the binder does not index (a[r] in a[i] <= a[r]) in a check is unsupported_annotation (in a contract it is accepted: forall_unindexed_read.c)
// Pre-registered from record 0061 item 10 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool le_at(int i, const int *a, int n, int r)
{
  return !(0 <= i && i < n) || a[i] <= a[r];
}

void use(const int *a, int n, int r)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && 0 <= r && r < n)
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  CERASTES_CHECK(cerastes_forall(le_at, a, n, r));
  (void)a;
  (void)n;
  (void)r;
}
