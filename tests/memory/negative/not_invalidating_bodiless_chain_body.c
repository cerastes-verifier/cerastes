// expect: error postcondition @ L27
// mode: c23, c++20
// twin: tests/memory/not_invalidating_bodiless_chain.c
// Pins: 0058 item 2: the same contract with a body that frees a->b->x and sets it to NULL: the outer pointer a->b is unchanged but the inner tie a->b->x == old(a->b->x) (under a->b != NULL) may not hold at the end of touch, so the implicit ensures reports one postcondition error (the second level of the path is checked, not only the first)
// Pre-registered from record 0058 item 2: one postcondition error at the closing brace of touch (observed with the ties written: build/m4.5d2b2d/emul/c2_emul.c)
#include <cerastes.h>
#include <stdlib.h>

struct leaf {
  int *x CERASTES_OWNING;
  int k;
};

struct mid {
  struct leaf *b CERASTES_OWNING;
  int m;
};

void touch(struct mid *a CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a))
{
  if (a->b != NULL) {
    free(a->b->x);
    a->b->x = NULL;
  }
}  // <- here

void mid_destroy(struct mid *a)
    CERASTES_REQUIRES(a == NULL || cerastes_owns_block(a))
{
  if (a != NULL) {
    if (a->b != NULL) {
      free(a->b->x);
      free(a->b);
    }
    free(a);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct mid *a = (struct mid *)calloc(1, sizeof(struct mid));
  if (a == NULL) {
    return -1;
  }
  a->b = (struct leaf *)calloc(1, sizeof(struct leaf));
  if (a->b == NULL) {
    mid_destroy(a);
    return -1;
  }
  a->b->x = (int *)malloc(sizeof(int));
  if (a->b->x == NULL) {
    mid_destroy(a);
    return -1;
  }
  *a->b->x = 1;
  int *t = a->b->x;
  touch(a);
  *t = 2;
  mid_destroy(a);
  return 0;
}
