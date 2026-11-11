// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/memory/marked_pointee_param.c
// Pins: 0058 item 4 (question 202): the marker still means "no value on entry": the callee reads p->n before writing it: uninitialized at the read
// Pre-registered from record 0058 item 4 (question 202): one uninitialized error at the read (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int fill(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int old = p->n;  // <- here
  p->n = 0;
  p->data = NULL;
  return old;
}
