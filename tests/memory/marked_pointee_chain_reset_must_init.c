// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit round 3 B1): the twin of marked_pointee_chain_reset.c with a must_init initializer: its ensures closes the chain, so the block of a->b is held again and reset verifies
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: verifies in both modes (the auditor's q7_chain_mi.c shape: no exit 3 with must_init; this exact program is the pin of the twin)
#include <cerastes.h>
#include <stdlib.h>
struct leaf { int *x CERASTES_OWNING; int k; };
struct mid { struct leaf *b CERASTES_OWNING; int m; };
void init(struct mid *a CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a))
{
  a->b = NULL;
  a->m = 0;
}
void reset(struct mid *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a))
{
  struct leaf *t = a->b;
  if (t != NULL) {
    free(t->x);
    free(t);
  }
  init(a);
}
