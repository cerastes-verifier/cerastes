// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2: cerastes::not_invalidating in the leading position on a parameter with a written contract and no owning: owns(*p) in and out, so the closure gives the block of p->data back, and the marker says it is the same block; the body writes the non-pointer member
// Pre-registered from record 0058 item 2: verifies in both modes (the written form, an explicit p->data == old(p->data), verifies today: build/m4.5d2b2d/emul/a3_leading_emul.c)
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
  p->n = 0;
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
  *t = 8;
  buf_destroy(b);
  return 0;
}
