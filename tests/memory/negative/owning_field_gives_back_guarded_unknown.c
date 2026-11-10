// expect: error lifetime @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_gives_back_guarded.c
// Pins: 0055 item 5: the same call with an unknown c: on the path c == 0 the callee did not give the cells back, so the failure is the caller's at the lifetime of s (the gives-back reads the cell only where it is held)
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: lifetime at the return of use ("the address of `s.data` may still be in use when its lifetime ends")
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

void maybe(struct buf *p, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->data == NULL)
    CERASTES_ENSURES(c != 0 ? (cerastes_owns(*p) && p->data == NULL) : 1);

int use(int c)
{
  struct buf s = { NULL, 0 };
  maybe(&s, c);
  return 0;  // <- here
}
