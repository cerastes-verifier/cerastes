// expect: rejected cerastes-annotation_side_effect @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 4 (audit round 2 issue 2): an assignment inside E is annotation_side_effect, reported at the quantifier
// Pre-registered from record 0061 item 4 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool g(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || ((i = 0) == 0 && a[i] > 0);
}

void f(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(g, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
