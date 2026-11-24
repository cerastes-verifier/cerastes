// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/forall_site_obligations.c
// Pins: 0061 items 6 and 7 (S45 (d) twin): a[i + 1] for i < n reads a[n], one past the range: the obligation of E is a permission error reported at the forall's clause (question 247)
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool next_nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i + 1] >= 0;
}

void shifted(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(next_nonneg, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
