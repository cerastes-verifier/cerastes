// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 11 (N5): a bodiless borrow of a chain a->b->x: the tie of 0054 item 2 at each level under the partner's guard (a->b == old(a->b) and, where a->b is not null, a->b->x == old(a->b->x)); the caller allocates the chain, calls it and destroys it
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes
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

void touch(struct mid *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a) && a->b == cerastes_old(a->b) && (a->b == NULL || a->b->x == cerastes_old(a->b->x)));

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
  if (a->b != NULL) {
    a->b->x = (int *)malloc(sizeof(int));
    if (a->b->x != NULL) {
      *a->b->x = 1;
    }
  }
  touch(a);
  mid_destroy(a);
  return 0;
}
