// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/not_invalidating_borrower.c
// Pins: 0058 item 2 ("With a body"): the body frees p->data and stores a fresh block, so the implicit ensures p->data == old(p->data) may not hold at the end of touch ("touch changes p->data, but p is cerastes::not_invalidating"); the caller, which relies on the marker, is unchanged and reports nothing
// Pre-registered from record 0058 item 2: one postcondition error at the closing brace of touch (observed on today's encoder with the tie written as an explicit ensures: build/m4.5d2b2d/emul/a2_replaces_emul.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_OWNING CERASTES_NOT_INVALIDATING)
{
  if (p != NULL) {
    free(p->data);
    p->data = (int *)malloc(sizeof(int));
    if (p->data != NULL) {
      *p->data = 0;
    }
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
