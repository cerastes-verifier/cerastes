// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/owning_field_result.c
// Pins: 0055 items 3, 4 (the value rule): the owning function stores the address of a local into the field of the structure it returns, which is not a block it holds (the field is set back to NULL right after, so that the closure's return row holds and the store is the only error); the store is where it is reported
// Pre-registered from record 0055 item 4: permission at the store (the frozen base, with the clause written by hand and the same NULL store, verifies this program: the value rule is new)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING struct buf *make(int v)
{
  struct buf *b = (struct buf *)malloc(sizeof(struct buf));
  if (b == NULL) {
    return NULL;
  }
  b->n = v;
  int x = v;
  b->data = &x;  // <- here
  b->data = NULL;
  return b;
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
  struct buf *b = make(4);
  if (b == NULL) {
    return -1;
  }
  buf_destroy(b);
  return 0;
}
