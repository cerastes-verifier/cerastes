// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 (S2, "Open"): a bodiless function that is not owning promises owns(*r) of its result, whose one owning field closes with a block the caller then frees; with one owning field nothing can alias the field's block with another the caller holds (build/m4.5d2b2b/s40/q/vi3.c on the frozen base)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (vi3.c, clause by hand: verified)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *get(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

void user(void)
{
  struct buf *s = get();
  if (s != NULL) {
    free(s->data);
  }
}
