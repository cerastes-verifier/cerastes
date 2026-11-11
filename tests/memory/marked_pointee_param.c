// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (question 202; flip of reject_owning_field_ref_to_uninit_param.c): ref_to_uninit on a pointer to a structure with an owning field is accepted: the marked site closes nothing, and the filler writes both members without reading them and leaves NULL in the owning field
// Pre-registered from record 0058 item 4 (question 202): verifies in both modes (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void fill(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 0;
  p->data = NULL;
}
