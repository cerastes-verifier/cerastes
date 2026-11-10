// expect: error permission @ L26
// mode: c23, c++20
// twin: tests/memory/owning_field_chain.c
// Pins: 0055 item 3: the second level of the closure is a clause too: the borrower frees the inner block and returns without setting a->b->x to NULL, "returns while owning field a->b->x does not hold its block"
// Pre-registered from record 0055 item 3: permission at the closing brace of reset (the return row of the inner clause)
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

void reset(struct mid *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a))
{
  if (a->b != NULL) {
    free(a->b->x);
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
  if (a->b != NULL) {
    a->b->x = (int *)malloc(sizeof(int));
    if (a->b->x != NULL) {
      *a->b->x = 1;
    }
  }
  reset(a);
  mid_destroy(a);
  return 0;
}
