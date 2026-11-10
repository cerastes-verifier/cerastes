// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 7 (S1): a written invariant owns(*p) is closed for every consumer: the loop frees the field's block and sets it to NULL each iteration, which the closure (p->data == NULL || owns_block(p->data)) in the invariant allows
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (the auditor's t/inv_drop.c with the free added)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void shed(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(p->data);
    p->data = NULL;
    i += 1;
  }
}
