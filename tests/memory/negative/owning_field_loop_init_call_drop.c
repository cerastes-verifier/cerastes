// expect: error leak @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_init_call.c
// Pins: 0055 items 3, 7 (S1): without the free the iteration sets the field to NULL while the initializer gave it a block, which no invariant carries: reported at the loop's back edge
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), leak at the loop body's closing brace (the auditor's t/loop_drop.c: leak at the back edge)
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
    p->data = NULL;
    i += 1;
  }  // <- here
}
