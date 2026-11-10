// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/owning_field_alias_fill.c
// Pins: 0055 items 4, 5 (a design limit, pinned): the same without the free: the callee took the block through the alias, which the named leaf s.data does not see, so the error is the end-of-function row at the closing line ("`use` may return while it still owns an allocation that its `ensures` does not hand back"), not the named-leaf row at the return
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: leak at the closing brace of use (the end-of-function row, not "owning field `s.data` still holds its block", which would be at the return)
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
  return 0;
}  // <- here
