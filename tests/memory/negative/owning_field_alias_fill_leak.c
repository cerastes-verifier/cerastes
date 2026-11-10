// expect: error leak @ L26
// mode: c23, c++20
// twin: tests/memory/owning_field_alias_fill.c
// Pins: 0055 items 4, 5 and 0056 item 4(a): the same without the free: the callee gave the block back through the alias a == &s, which the named leaf s.data now sees (the guarded update), so the error is the named-leaf row at the return ("owning field `s.data` still holds its block"); before 0056 it was the end-of-function row at the closing line (the plan's Flip B: L27 became L26)
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: leak at the closing brace of use (the end-of-function row, not "owning field `s.data` still holds its block", which would be at the return); re-registered in M4.5d2b2c from record 0056 item 4(a): the named-leaf row at the return (the plan's Flip B)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

void buf_fill(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = 1;
  }
}

int use(void)
{
  struct buf s = { NULL, 0 };
  struct buf *a = &s;
  buf_fill(a);
  return 0;  // <- here
}
