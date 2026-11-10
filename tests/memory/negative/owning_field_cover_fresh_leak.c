// expect: error leak @ L31
// mode: c23, c++20
// twin: tests/memory/owning_field_cover.c
// Pins: 0055 item 3 ("Covered atoms"): a function whose output atom owns(*r) is covered on some paths returns a fresh structure on another (return x;): the block of its owning field is a block the caller can no longer receive, so the function leaks it: reported at the closing line (s3_lost2.c)
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): record 0055 item 3: leak at the closing brace of pick
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *get(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

struct buf *pick(int c)
    CERASTES_ENSURES(r, cerastes_owns(*r))
{
  static struct buf s;
  struct buf *x = get();
  if (x == NULL) {
    return &s;
  }
  if (c != 0) {
    free(x->data);
    x->data = NULL;
    return &s;
  }
  return x;
}  // <- here

int user(void)
{
  struct buf *p = pick(0);
  return p->n;
}
