// expect: error leak @ L20
// mode: c23, c++20
// twin: tests/memory/marked_pointee_param_second.c
// Pins: 0058 item 4 (question 202): the same with the owning field second: the block left in the field is the callee's leak
// Pre-registered from record 0058 item 4 (question 202): one leak at the end of fill (observed on the Part F build with question 202 applied)
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
  p->data = (int *)malloc(sizeof(int));
}  // <- here
