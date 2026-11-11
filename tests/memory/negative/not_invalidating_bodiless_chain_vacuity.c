// expect: error check @ L56
// mode: c23, c++20
// twin: tests/memory/not_invalidating_bodiless_chain.c
// Pins: 0058 item 2 (audit round 2, S1): the two-level bodiless marked contract is consistent, not vacuous: after touch the caller's a->b->x is the block t points into (not NULL), so the false fact a->b->x == NULL fails its check
// Pre-registered from record 0058 item 2 (audit round 2, S1): one check error at the check
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
  CERASTES_CHECK(a->b->x == NULL);  // <- here
  mid_destroy(a);
  return 0;
}
