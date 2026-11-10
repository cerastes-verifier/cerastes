// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 2, 3 (question 191, "Recursively"): an owning field whose pointee structure has an owning field, two levels deep (a->b->x): owns(*a) closes with a->b, which closes with a->b->x; the borrower frees the inner block and sets the inner field to NULL, the destructor consumes owns_block(a) with both
// Pre-registered from record 0055 items 2, 3: verifies in both modes (pre-agreed cut 3 rejects chains instead, naming M4.5f2; then these two tests are re-expected as pins, an owner decision)
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
    a->b->x = NULL;
  }
}

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
