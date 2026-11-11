// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit round 1 S3, round 3 S2): a marked owning field q whose pointee has an owning field: the automatic closure closes nothing below q, so a filler that writes q->x and q->k verifies
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: verifies in both modes (audit3/s3_auto.c without its caller)
#include <cerastes.h>
#include <stdlib.h>
struct T { int *x CERASTES_OWNING; int k; };
struct S { struct T *q CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void fill(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q))
{
  if (p->q != NULL) {
    p->q->x = NULL;
    p->q->k = 0;
  }
}
