// expect: error leak @ L20
// mode: c23, c++20
// twin: tests/memory/owning_field_merge.c
// Pins: 0055 item 3 ("Merging"): the merged clause is a real site, not a no-op: the block it gives is dropped by setting the field to NULL, reported by the end-of-function row
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), record 0055 items 3, 4: leak at the closing brace
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
  p->data = NULL;
}  // <- here
