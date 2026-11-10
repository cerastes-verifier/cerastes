// expect: error leak @ L25
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_shed.c
// Pins: 0055 items 3, 7 (S1): the loop of t/inv_drop.c: the body only sets the field to NULL, dropping the block the invariant carries: leak at the back edge
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), leak at the loop body's closing brace (t/inv_drop.c: leak)
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
    p->data = NULL;
    i += 1;
  }  // <- here
}
