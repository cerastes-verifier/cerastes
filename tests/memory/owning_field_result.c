// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("an owning function's result"; 0047 item 5): the result r == NULL || owns_block(r) carries the closure of its owning field; the body fills the field from malloc and the caller destroys the structure with its block
// Pre-registered from record 0055 item 3: verifies in both modes (the body is checked against the closure clause: it holds the block it stored)
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
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = v;
  }
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
