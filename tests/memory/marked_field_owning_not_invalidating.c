// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 items 2 and 4 (audit round 3 S1): an owning parameter with not_invalidating on a structure whose owning field is marked: the implicit tie is spliced under p == NULL || ..., and a tie under a guard is recognized, so the closure keeps the flags and the caller reads the buffer it wrote after the call
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: verifies in both modes after S1's fix (audit3/k6_owning_ni.c: uninitialized at the caller's read before)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *q CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void fill(struct buf *p CERASTES_OWNING CERASTES_NOT_INVALIDATING)
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
    int y = *b->q;
    b->n = y;
  }
  free(b->q);
  free(b);
  return 0;
}
