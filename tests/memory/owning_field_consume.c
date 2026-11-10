// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("a plain block site on a pointer to such a structure"): a consumer of owns_block(p) with no guard takes the field's block with it (the closure p->data == NULL || owns_block(p->data) has no partner), frees both, and the caller no longer holds either
// Pre-registered from record 0055 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void dispose(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
  free(p->data);
  free(p);
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)malloc(sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->n = 0;
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = 1;
  }
  dispose(b);
  return 0;
}
