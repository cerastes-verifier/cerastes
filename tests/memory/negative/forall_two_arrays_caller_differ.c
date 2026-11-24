// expect: error precondition @ L27
// mode: c23, c++20
// twin: tests/memory/forall_two_arrays.c
// Pins: 0061 item 6 (S45 (b) twin): the arrays differ at the last slot: the precondition row of the two-class clause fails at the call
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool same_at(int i, const int *a, const int *b, int n)
{
  return !(0 <= i && i < n) || a[i] == b[i];
}

int diff(const int *a, const int *b, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_reads_range(b, n) && n > 0 && cerastes_forall(same_at, a, b, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && cerastes_reads_range(b, n) && r == 0)
{
  (void)n;
  return a[0] - b[0];
}

int caller(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  int b[3] = {1, 2, 4};
  return diff(a, b, 3);
}
