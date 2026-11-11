// expect: error leak @ L20
// mode: c23, c++20
// twin: tests/memory/marked_pointee_param.c
// Pins: 0058 item 4 (question 202): a callee that leaves a block in the owning field of a marked pointee: the marked site closes nothing, so nothing hands the block back and the callee's own end row reports it: leak at the closing brace of fill
// Pre-registered from record 0058 item 4 (question 202): one leak at the end of fill (observed on the Part F build with question 202 applied)
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
  p->data = (int *)malloc(sizeof(int));
}  // <- here
