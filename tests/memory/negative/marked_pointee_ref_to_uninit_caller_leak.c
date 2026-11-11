// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/marked_pointee_must_init.c
// Pins: 0058 item 4 (question 202, "the price"): the same caller with a ref_to_uninit callee: its ensures cannot name p->data (only must_init promises a value), so after the call the caller does not know s.data and its end row reports a leak although the callee set NULL
// Pre-registered from record 0058 item 4 (question 202): one leak at the end of use (the sound direction) (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void init(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 0;
  p->data = NULL;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct buf s = { NULL, 1 };
  init(&s);
  return 0;  // <- here
}
