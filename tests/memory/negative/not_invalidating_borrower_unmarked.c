// expect: error permission @ L45
// mode: c23, c++20
// twin: tests/memory/not_invalidating_borrower.c
// Pins: 0058 item 2: the same program without cerastes::not_invalidating: nothing says touch leaves p->data alone, so the block the closure gives back is not known to be the one t points into, and the write through t is a permission error (the one reason: the marker is gone)
// Pre-registered from record 0058 item 2: one permission error at the write through t (observed on today's encoder: build/m4.5d2b2d/emul/a1_unmarked.c, "writes `*t` without owning it")
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_OWNING)
{
  if (p != NULL) {
    p->n = 0;
  }
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
  b->data = (int *)malloc(sizeof(int));
  if (b->data == NULL) {
    buf_destroy(b);
    return -1;
  }
  *b->data = 7;
  int *t = b->data;
  touch(b);
  *t = 8;  // <- here
  buf_destroy(b);
  return 0;
}
