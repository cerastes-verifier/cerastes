// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/not_invalidating_borrower.c
// Pins: 0058 item 2 (the implicit clause is tied under the framing atom's guard, p == NULL || p->data == old(p->data), never under the site's own null test): the body frees p->data and sets it to NULL, which a tie under p->data != NULL would accept vacuously; the end of touch reports the implicit ensures
// Pre-registered from record 0058 item 2 (as revised): one postcondition error at the closing brace of touch (observed with the tie written as an explicit ensures: build/m4.5d2b2d/emul/j1_emul.c)
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
    p->data = NULL;
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
