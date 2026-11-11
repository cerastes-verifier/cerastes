// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4: an owning field marked ref_to_uninit: the structure is allocated, its buffer is allocated and not yet written, and it is passed to a filler that writes the buffer: the closure of the marked field claims no flags on entry, so the call needs no value in *p->data
// Pre-registered from record 0058 item 4: verifies in both modes (the registration and reader of Part F)
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
  fill(b);
  buf_destroy(b);
  return 0;
}
