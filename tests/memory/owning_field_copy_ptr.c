// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: *q = *p; through pointers is its leaf stores (the value rule at each owning leaf); q's old block is freed first and p->data is set to NULL after the copy, so the two structures do not both hand back one block
// Pre-registered from record 0055 item 6 (copies and values): verifies in both modes (S40 (e) e_ptr_copy.c on the frozen base, by hand: verified)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void move(struct buf *p, struct buf *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q))
{
  free(q->data);
  *q = *p;
  p->data = NULL;
}
