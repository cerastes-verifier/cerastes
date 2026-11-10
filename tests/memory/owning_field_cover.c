// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("Covered atoms"): a function that returns &s of its static struct local under ensures(r, owns(*r)): the output atom is covered by the implicit owns(s) and carries no closure, so a caller writes through the result and calls the function again
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): cov2_own.c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *where(void)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static struct buf s;
  return &s;
}

void user(void)
{
  struct buf *p = where();
  p->n = 1;
  struct buf *p2 = where();
  (void)p2;
}
