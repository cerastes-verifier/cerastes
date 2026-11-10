// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/owning_field_nested.c
// Pins: 0055 item 3: the nested leaf o->t.q is closed like a direct one: the borrower frees its block and returns without setting it to NULL, "returns while owning field o->t.q does not hold its block"
// Pre-registered from record 0055 item 3: permission at the closing brace of reset (the return row of the clause)
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
}  // <- here

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
