// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_replace.c
// Pins: 0055 item 7: the same loop without the free: an iteration ends owning the block it replaced, which the loop does not carry; no row at the store through a pointer (item 4), so the loop's back-edge row reports it at the loop body's closing brace
// Pre-registered from record 0055 item 7 (loops): leak at the loop body's closing brace (S40 (g) g1_drop.c on the frozen base, clause by hand: the same position)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void churn(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->data = (int *)malloc(sizeof(int));
    if (p->data != NULL) {
      *p->data = i;
    }
    i += 1;
  }  // <- here
}
