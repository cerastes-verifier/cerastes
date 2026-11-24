// expect: rejected cerastes-unsupported_annotation @ L19
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: STAGING PIN S1 of M4.5e1c (record 0061 item 5: check.cpp refuses a Forall in a loop invariant until Part C): deleted by Part C, which gives the program its verifying form (forall_fill_local.c and its siblings)
// Pre-registered from record 0061 item 1 (Part B), before the code
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
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(nonneg, a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
}
