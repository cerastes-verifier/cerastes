// expect: rejected cerastes-unsupported_expression @ L16
// mode: c++20
// twin: tests/memory/pure_keep_fact_cell.c
// Pins: 0040 item 2 (audit round 3 R3-S3): C++'s implicit multi-level qualification conversion int ** to const int *const * at an application is rejected like the same conversion written as a cast
// Pre-registered from record 0062 (Part C), before the code: rejected. C++ only; from the auditor's probe q5_ptrptr_implicit.cpp
#include <cerastes.h>

CERASTES_PURE int nn(const int *const *pp)
    CERASTES_REQUIRES(cerastes_reads(*pp))
{
  return *pp != 0;
}

int f(int **qq)
    CERASTES_REQUIRES(cerastes_reads(*qq) && *qq != 0)
    CERASTES_ENSURES(r, cerastes_reads(*qq) && nn(qq) == 1)
{
  return 0;
}
