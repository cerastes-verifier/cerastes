// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_replace_written.c
// Pins: 0055 item 7: the written clause does not hide the drop: without the free an iteration ends owning the block it replaced, reported at the loop body's closing brace as in owning_field_loop_drop.c
// Pre-registered from record 0055 item 7: leak at the loop body's closing brace (S40 (g) g1_drop.c on the frozen base, clause by hand, which is this invariant verbatim: the same position)
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
  CERASTES_INVARIANT(cerastes_owns(*p) && (p->data == NULL || cerastes_owns_block(p->data)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p->data = (int *)malloc(sizeof(int));
    if (p->data != NULL) {
      *p->data = i;
    }
    i += 1;
  }  // <- here
}
