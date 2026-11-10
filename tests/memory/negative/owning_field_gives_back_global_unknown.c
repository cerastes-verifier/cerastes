// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/owning_field_gives_back_global.c
// Pins: 0055 item 5: without c != 0 required the wrapper cannot return owns(g): where c == 0 the callee did not give the global back ("returns without owning `g`, which its contract promises")
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: permission at the return of wrap
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

static struct buf g = { NULL, 0 };

void maybe(int c)
    CERASTES_REQUIRES(cerastes_owns(g) && g.data == NULL)
    CERASTES_ENSURES(c != 0 ? (cerastes_owns(g) && g.data == NULL) : 1);

int wrap(int c)
    CERASTES_REQUIRES(cerastes_owns(g) && g.data == NULL)
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  if (g.data != NULL) {
    return 1;
  }
  maybe(c);
  return 0;  // <- here
}

int main(void)
{
  return wrap(1);
}
