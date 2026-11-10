// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2: cerastes::not_invalidating beside owning on a parameter, trailing position, with a body: the body writes a non-pointer member, so the implicit ensures (p == NULL || p->data == old(p->data)) holds at the return; the caller holds t = b->data across the call and writes *t after it
// Pre-registered from record 0058 item 2 ("What the caller gains"): verifies in both modes (the written form, owning plus a tie in an explicit contract, verifies today: build/m4.5d2b2d/emul/a1_marked_emul.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_OWNING CERASTES_NOT_INVALIDATING)
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
  *t = 8;
  buf_destroy(b);
  return 0;
}
