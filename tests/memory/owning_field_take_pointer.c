// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 4, 11 (N3): through a pointer the same order is accepted: there is no dropped-block row at a store through a pointer (S40 (d)), so taking the block into a plain local, setting the field to NULL and freeing it verifies (memory safety)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (S40 (d) d_local_take.c on the frozen base: verified)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void take(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *t = p->data;
  p->data = NULL;
  free(t);
}
