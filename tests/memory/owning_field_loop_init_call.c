// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 7 (S1): a loop whose body gives the structure to a bodied must_init initializer (its ensures closes: the block it stores is a given site) and then frees that block and sets the field back to NULL, under the invariant p->data == NULL
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (the auditor's t/loop_drop.c with the free added, clause by hand on the frozen base)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_init(struct buf *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 1;
  }
  p->n = 0;
}

void rounds(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->data == NULL)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && p->data == NULL)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    buf_init(p);
    free(p->data);
    p->data = NULL;
    i += 1;
  }
}
