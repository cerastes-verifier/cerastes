// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 (S1): the closure is seen by the sequencing rules: a read through a pointer taken from the owning field is ordered before the call that frees the block, in a separate statement
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int refill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  free(p->data);
  p->data = NULL;
  return 0;
}

int total(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->data != NULL)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int *t = p->data;
  int x = *t;
  int y = refill(p);
  return x + y;
}
