// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_leaf.c
// Pins: 0055 item 3: owns(p->data) gives the cell of the field and its block, nothing else of the structure; a write to p->n needs a permission the borrower does not hold
// Pre-registered from record 0055 item 3: permission at the write
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void renew(struct buf *p, int v)
    CERASTES_REQUIRES(cerastes_owns(p->data))
    CERASTES_ENSURES(cerastes_owns(p->data))
{
  free(p->data);
  p->data = NULL;
  p->n = v;  // <- here
}

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
  renew(b, 1);
  buf_destroy(b);
  return 0;
}
