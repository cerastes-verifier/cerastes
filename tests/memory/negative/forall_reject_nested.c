// expect: rejected cerastes-unsupported_annotation @ L20
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 1 (M4.8, question 249): a forall inside E is unsupported_annotation
// Pre-registered from record 0061 item 1 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

CERASTES_PURE bool outer(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || cerastes_forall(nonneg, a, n);
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(outer, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
