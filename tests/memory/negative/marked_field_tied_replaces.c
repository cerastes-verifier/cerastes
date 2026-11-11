// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/marked_field_untied_replaces.c
// Pins: 0058 item 4 (audit B1): the same callee with the tie p->data == old(p->data) written: the body replaces the buffer, so the tie may not hold: one postcondition error at the closing brace of reset and no internal check (the kept row and the tie are one atomic assertion)
// Pre-registered from record 0058 item 4 (audit B1, the il-analyst's rows): one postcondition error at the closing brace of reset (partF2/f3_tied_replaces.c)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
void reset(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->data == cerastes_old(p->data))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
}  // <- here
int use(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  reset(&s);
  free(s.data);
  return 0;
}
