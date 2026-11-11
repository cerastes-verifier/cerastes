// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit round 3 S2): the same contract with the closure written, p->q == NULL || owns_block(p->q): a written closure-shaped site on a marked field behaves as the automatic one and closes nothing below q, so the filler verifies (audit3/s3_written.c gave "reads p->q->x without owning it" before the fix)
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: verifies in both modes after S2's fix
#include <cerastes.h>
#include <stdlib.h>
struct T { int *x CERASTES_OWNING; int k; };
struct S { struct T *q CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void fill(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && (p->q == NULL || cerastes_owns_block(p->q)))
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && (p->q == NULL || cerastes_owns_block(p->q)))
{
  if (p->q != NULL) {
    p->q->x = NULL;
    p->q->k = 0;
  }
}
