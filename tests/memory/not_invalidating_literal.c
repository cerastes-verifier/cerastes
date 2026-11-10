// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2: the attribute spelt out, [[cerastes::not_invalidating]], in the leading position (touch_a) and the trailing one (touch_b); the macro stands behind a local definition so that the file also compiles natively; the caller keeps t = b->data across both calls
// Pre-registered from record 0058 item 2 ("in either position"): verifies in both modes
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

void touch_b(struct buf *p NOT_INV)
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
  *t = 9;
  buf_destroy(b);
  return 0;
}
