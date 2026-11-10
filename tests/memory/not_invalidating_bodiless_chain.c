// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2: a bodiless marked borrower of a chain a->b->x: the implicit ensures ties each loaded pointer on the place's path (a->b == old(a->b), and a->b->x == old(a->b->x) where a->b is not null), so no tie is written at either level; the caller holds t = a->b->x across the call and writes *t after it
// Pre-registered from record 0058 item 2 ("a two-level chain"): verifies in both modes (the written form, owning_field_bodiless_chain.c with the same caller shape, verifies today: build/m4.5d2b2d/emul/c1_emul.c)
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
    CERASTES_ENSURES(cerastes_owns(*a));

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
