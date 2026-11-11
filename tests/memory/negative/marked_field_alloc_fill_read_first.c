// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/memory/marked_field_alloc_fill.c
// Pins: 0058 item 4: the filler reads the buffer before writing it: a borrower cannot read what the marked closure promised no value for: uninitialized at the read in the callee
// Pre-registered from record 0058 item 4: one uninitialized error at the read of *p->data in fill (observed on the Part F build: build/m4.5d2b2d/emul3/f0_read.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

void fill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (p->data != NULL) {
    int x = *p->data;  // <- here
    p->n = x;
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
  fill(b);
  buf_destroy(b);
  return 0;
}
