// expect: rejected cerastes-unsupported_annotation @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_replace_written.c
// Pins: 0055 items 3, 11: another block-site shape on the owning field beside owns(*p) in an invariant (here with no null alternative) is rejected: owns(*p) already includes the block of owning field p->data
// Pre-registered from record 0055 item 7 (loops): rejected unsupported_annotation at the invariant clause, as reject_owning_field_beside_block.c does for a requires (no base form; the category and line are the pin's guess for an invariant: the frozen base accepts the program)
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
  CERASTES_INVARIANT(cerastes_owns(*p) && cerastes_owns_block(p->data))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(p->data);
    p->data = (int *)malloc(sizeof(int));
    if (p->data != NULL) {
      *p->data = i;
    }
    i += 1;
  }
}
