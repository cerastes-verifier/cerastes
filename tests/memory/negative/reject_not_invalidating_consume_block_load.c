// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/not_invalidating_borrower.c
// Pins: 0058 item 2 ("The check on the contract"): a block site through a load on the marked parameter: requires owns_block(p->q) with only owns(*p) in the ensures: the block of p->q is taken and not returned (the field is not owning, so no closure returns it): unsupported_annotation at the parameter. Without the marker this verifies (build/m4.5d2b2d/emul2/cb.c)
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter ("takes owns_block(p->q) without returning it")
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int *q;
  int n;
};

void drop(struct S *p CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns_block(p->q))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->q);
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *s = (struct S *)calloc(1, sizeof(struct S));
  if (s == NULL) {
    return -1;
  }
  s->q = (int *)malloc(sizeof(int));
  if (s->q == NULL) {
    free(s);
    return -1;
  }
  *s->q = 0;
  drop(s);
  free(s);
  return 0;
}
