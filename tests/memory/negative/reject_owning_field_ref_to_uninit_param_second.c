// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 items 3, 11 (audit round 2): the rejection of ref_to_uninit on a pointer to a structure with an owning field names the owning field `data` although it is not the first member (int n comes first)
// Pre-registered from audit round 2 on Part A (record 0055 as revised, 78838ca), the edg-backend's fix to the message: rejected unsupported_annotation at the parameter; the message must say `the owning field `data``
#include <cerastes.h>
#include <stdbool.h>
#include <stdlib.h>

struct buf {
  int n;
  int *data CERASTES_OWNING;
};

void fill(struct buf *p CERASTES_REF_TO_UNINIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 0;
  p->data = NULL;
}
