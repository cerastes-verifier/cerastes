// expect: error permission @ L22
// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/owning_field_bodiless_chain.c
// Pins: 0055 item 3 (N5): the inner tie is missing: a bodiless borrow could return another block in a->b->x, so touch__wf fails at the ensures clause (as owning_field_bodiless_untied.c does for the outer level)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), two permission errors at the ensures clause (the count is a best reading, from owning_field_bodiless_untied.c; to be confirmed by the first run, question 160)
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
    CERASTES_ENSURES(cerastes_owns(*a) && a->b == cerastes_old(a->b));  // <- here

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
