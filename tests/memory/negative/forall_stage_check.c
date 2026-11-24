// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: STAGING PIN S2 of M4.5e1c (record 0061 item 5: check.cpp refuses a Forall in a check until Part C): deleted by Part C
// Pre-registered from record 0061 item 1 (Part B), before the code
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
  CERASTES_CHECK(cerastes_forall(nonneg, a, n));
}
