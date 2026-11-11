// expect: error invalid-free @ L30
// expect: error uninitialized @ L30
// mode: c23, c++20
// twin: tests/memory/marked_field_auto_site_fill.c
// Pins: 0058 item 4 (audit round 3 S2): the caller of the automatic-closure filler frees s->q->x, which after the call it holds no promise about: invalid-free and uninitialized (the price of a marked field: the sound direction)
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: invalid-free and uninitialized at the free of s->q->x (observed: audit3/s3_auto.out)
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
int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *s = (struct S *)calloc(1, sizeof(struct S));
  if (s == NULL) {
    return -1;
  }
  s->q = (struct T *)malloc(sizeof(struct T));
  fill(s);
  if (s->q != NULL) {
    free(s->q->x);  // <- here
  }
  free(s->q);
  free(s);
  return 0;
}
