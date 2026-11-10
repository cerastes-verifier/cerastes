// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 4, 5: a callee fills a struct local through an alias, a = &s: the framed update after the call gives the leaf back to s.data under its ghost, and the function frees s.data
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: verifies in both modes
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
  free(s.data);
  return 0;
}
