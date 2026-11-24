// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/pure_forall_check_sum.c
// Pins: 0062 item 3 (audit round 2 R2-S5): a read through a loaded pointer in a read pure body (**pp) is rejected: reads go through the function's pointer parameters only
// Pre-registered from record 0062 (Part B), before the code: rejected. from the auditor's probe p23_ptrptr.c; written before edg-backend's rejection lands: the tag and the line are question-160 candidates
#include <cerastes.h>

CERASTES_PURE int dd(int *const *pp)
    CERASTES_REQUIRES(cerastes_reads(*pp))
{
  return **pp;
}

int use(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*q))
{
  int *const *pq = &q;
  CERASTES_CHECK(dd(pq) == *q);
  return *pq == 0;
}
