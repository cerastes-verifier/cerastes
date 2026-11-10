// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2 ("It names pointers, never values"): the marker ties pointers only, so the value of a non-pointer member is promised by a written clause (p->n == old(p->n)); the body writes the cell behind p->data, which the marker allows (it changes a value, not a pointer), and the caller's check of b->n holds
// Pre-registered from record 0058 item 2: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->n == cerastes_old(p->n))
{
  if (p->data != NULL) {
    *p->data = 0;
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
  b->n = 5;
  touch(b);
  CERASTES_CHECK(b->n == 5);
  buf_destroy(b);
  return 0;
}
