// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("Merging and allocators"): the user writes the exact platform clause beside owns(*p) in the requires: it takes the closure's place, and the ensures (with the closure added) is met after the free
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), z_merge.c: verifies in both modes
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void clear(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && (p->data == NULL || cerastes_owns_block(p->data)))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = NULL;
}
