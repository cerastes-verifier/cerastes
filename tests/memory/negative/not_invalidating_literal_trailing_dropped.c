// expect: error permission @ L60
// mode: c23, c++20
// twin: tests/memory/not_invalidating_literal.c
// Pins: 0058 item 2: touch_b loses its trailing marker, so after touch_b the caller no longer knows p->data is the block t points into: the write through t after touch_b is a permission error, and the leading marker of touch_a still works (the write before it is accepted)
// Pre-registered from record 0058 item 2: one permission error, at the write through t after touch_b
#include <cerastes.h>
#include <stdlib.h>

#ifdef __CERASTES__
#  define NOT_INV [[cerastes::not_invalidating]]
#else
#  define NOT_INV
#endif

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch_a(NOT_INV struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 1;
}

void touch_b(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 2;
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
  touch_a(b);
  *t = 8;
  touch_b(b);
  *t = 9;  // <- here
  buf_destroy(b);
  return 0;
}
