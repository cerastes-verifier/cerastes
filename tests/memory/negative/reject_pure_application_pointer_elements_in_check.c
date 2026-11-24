// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/pure_keep_fact_cell.c
// Pins: 0062 item 7 (audit round 3 R3-S2; M4.5e2): an application of a function that reads pointer elements in a check is unsupported_annotation (the same application in the contract, two lines above, is accepted)
// Pre-registered from record 0062 (Part C), before the code: rejected. from the auditor's probe q7_ptr_range.c
#include <cerastes.h>

CERASTES_PURE int first_nonnull(int *const *pp, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(pp, n))
{
  return pp[0] != 0;
}

int f(int **qq, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(qq, n) && first_nonnull(qq, n) == 1)
    CERASTES_ENSURES(r, cerastes_reads_range(qq, n))
{
  CERASTES_CHECK(first_nonnull(qq, n) == 1);
  return 0;
}
