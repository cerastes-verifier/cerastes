// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/owning_field_param.c
// Pins: 0055 item 3: the closure clause of an owning parameter is in the postcondition too: the borrower frees the field's block and returns without setting it to NULL, "returns while owning field p->data does not hold its block"
// Pre-registered from record 0055 item 3: permission at the closing brace of clear (the return row of the clause)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void clear(struct buf *p CERASTES_OWNING)
{
  if (p != NULL) {
    free(p->data);
  }
}  // <- here

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = 1;
  }
  clear(b);
  buf_destroy(b);
  return 0;
}
