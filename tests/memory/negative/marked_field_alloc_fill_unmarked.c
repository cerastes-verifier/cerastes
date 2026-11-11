// expect: error uninitialized @ L40
// mode: c23, c++20
// twin: tests/memory/marked_field_alloc_fill.c
// Pins: 0058 item 4: the same program with the field not marked: the closure is plain (0055 item 3) and the call needs the buffer to hold a value: uninitialized at the call of fill (the one reason: the marker is gone)
// Pre-registered from record 0058 item 4: one uninitialized error at the call of fill (observed: build/m4.5d2b2d/emul3/f0_unmarked.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void fill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (p->data != NULL) {
    *p->data = 7;
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
  fill(b);  // <- here
  buf_destroy(b);
  return 0;
}
