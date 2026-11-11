// expect: error uninitialized @ L28
// mode: c23, c++20
// twin: tests/memory/marked_field_guarded_tie.c
// Pins: 0058 item 4 (audit round 3 S1): the same contract without the tie: the guarded site is untied and claims no flags, so the caller's read is uninitialized
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: one uninitialized error at the caller's read
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *q CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void fill(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(p == NULL || (cerastes_owns(*p)))
{
  if (p != NULL && p->q != NULL) { *p->q = 1; }
}
int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->q = (int *)malloc(sizeof(int));
  if (b->q != NULL) {
    *b->q = 7;
  }
  fill(b);
  if (b->q != NULL) {
    int y = *b->q;  // <- here
    b->n = y;
  }
  free(b->q);
  free(b);
  return 0;
}
