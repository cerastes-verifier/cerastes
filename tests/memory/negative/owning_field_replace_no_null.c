// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_replace.c
// Pins: 0055 item 3 (the return row of the closure clause): the borrower frees the block of p->data and returns without setting the field to NULL or giving it a new block, so the postcondition's p->data == NULL || owns_block(p->data) fails: "returns while owning field p->data does not hold its block"
// Pre-registered from record 0055 item 3: permission at the closing brace of drop (today's return row)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void drop(struct buf *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  (void)v;
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
  drop(b, 3);
  buf_destroy(b);
  return 0;
}
