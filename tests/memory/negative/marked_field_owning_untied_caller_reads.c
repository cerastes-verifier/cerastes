// expect: error uninitialized @ L26
// mode: c23, c++20
// twin: tests/memory/marked_field_owning_not_invalidating.c
// Pins: 0058 items 2 and 4 (audit round 3 S1): the same program without not_invalidating: nothing ties p->q, an untied site claims no flags, and the caller's read after the call is uninitialized
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: one uninitialized error at the caller's read (observed on the pre-fix build with k6's marker present; unchanged by the fix)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *q CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void fill(struct buf *p CERASTES_OWNING)
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
