// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (question 202; flip of reject_owning_field_ref_to_uninit_param_second.c): the owning field is not the first member (int n comes first): the marked pointee is accepted as in marked_pointee_param.c
// Pre-registered from record 0058 item 4 (question 202): verifies in both modes (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int n;
  int *data CERASTES_OWNING;
};

void fill(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 0;
  p->data = NULL;
}
