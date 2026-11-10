// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5: the same guarded gives-back for a struct global's leaf under owns(g): a wrapper holds g, calls maybe(c) with c != 0 required and returns owns(g)
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

static struct buf g = { NULL, 0 };

void maybe(int c)
    CERASTES_REQUIRES(cerastes_owns(g) && g.data == NULL)
    CERASTES_ENSURES(c != 0 ? (cerastes_owns(g) && g.data == NULL) : 1);

int wrap(int c)
    CERASTES_REQUIRES(cerastes_owns(g) && g.data == NULL && c != 0)
    CERASTES_ENSURES(r, cerastes_owns(g))
{
  if (g.data != NULL) {
    return 1;
  }
  maybe(c);
  return 0;
}

int main(void)
{
  return wrap(1);
}
