// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/marked_pointee_local.c
// Pins: 0058 item 4 (question 202): the same program with a wrong ensures
// Pre-registered from record 0058 item 4 (question 202): one postcondition error at the return (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct buf s = { NULL, 1 };
  CERASTES_REF_TO_UNINIT struct buf *q = &s;
  q->n = 0;
  return s.n;  // <- here
}
