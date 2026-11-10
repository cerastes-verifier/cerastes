// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 2, 3: an owning field in a by-value member (p->t.q): owns(*o) covers it, the closure clause names the leaf through the member, and the borrower frees the block and sets the leaf to NULL
// Pre-registered from record 0055 items 2, 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct inner {
  int *q CERASTES_OWNING;
  int k;
};

struct outer {
  struct inner t;
  int m;
};

void reset(struct outer *o)
    CERASTES_REQUIRES(cerastes_owns(*o))
    CERASTES_ENSURES(cerastes_owns(*o))
{
  free(o->t.q);
  o->t.q = NULL;
}

void outer_destroy(struct outer *o)
    CERASTES_REQUIRES(o == NULL || cerastes_owns_block(o))
{
  if (o != NULL) {
    free(o->t.q);
    free(o);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct outer *o = (struct outer *)calloc(1, sizeof(struct outer));
  if (o == NULL) {
    return -1;
  }
  o->t.q = (int *)malloc(sizeof(int));
  if (o->t.q != NULL) {
    *o->t.q = 1;
  }
  reset(o);
  outer_destroy(o);
  return 0;
}
