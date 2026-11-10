// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5: a callee whose ensures gives the struct local back only where c != 0, called with c == 1: the cell is read after the call only where it is held again, and s.data is NULL before and after
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

void maybe(struct buf *p, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->data == NULL)
    CERASTES_ENSURES(c != 0 ? (cerastes_owns(*p) && p->data == NULL) : 1);

int use(void)
{
  struct buf s = { NULL, 0 };
  maybe(&s, 1);
  return 0;
}
