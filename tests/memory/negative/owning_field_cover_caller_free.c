// expect: error invalid-free @ L27
// expect: error invalid-free @ L27
// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/owning_field_cover.c
// Pins: 0055 item 3 ("Covered atoms"): the caller of a function returning &s neither receives nor frees r->data's block: the free is not of an allocation it owns (cover.c; s3_lost2_df.c's second twin is split from the leak of pick so that each twin fails for one reason)
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): record 0055 item 3: invalid-free at the free
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
  free(p->data);  // <- here
  p->data = NULL;
}
