// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/owning_field_lifecycle.c
// Pins: 0055 items 3, 11: ref_to_uninit on a parameter whose pointee structure has an owning field is rejected (d2b2c's strict rules decide it): a cell without a value owns nothing, but the structure's closure would claim a block for the field
// Pre-registered from record 0055 item 11: rejected unsupported_annotation at the function (the declaration line, as reject_marker_ref_no_owns.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void fill(struct buf *p CERASTES_REF_TO_UNINIT)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->data = NULL;
  p->n = 0;
}

int ident(int a)
{
  return a;
}
