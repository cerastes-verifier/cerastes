// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/memory/not_invalidating_leading.c
// Pins: 0058 item 2 ("With a body"): the body moves the block of p->data into a local, sets the field to NULL and frees the block: the contract's closure still holds (a NULL field closes with nothing), but p->data == old(p->data) does not, so the end of touch reports the implicit ensures
// Pre-registered from record 0058 item 2: one postcondition error at the closing brace of touch (observed on today's encoder with the tie written: build/m4.5d2b2d/emul/a4_takes_emul.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(CERASTES_NOT_INVALIDATING struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *old_data = p->data;
  p->data = NULL;
  free(old_data);
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
  if (b->data == NULL) {
    buf_destroy(b);
    return -1;
  }
  *b->data = 7;
  int *t = b->data;
  touch(b);
  *t = 8;
  buf_destroy(b);
  return 0;
}
