// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (question 202): the initializer of a structure with an owning field is must_init: its ensures may name the owning field's value, so the caller's struct local knows data == NULL afterwards and ends without a leak
// Pre-registered from record 0058 item 4 (question 202): verifies in both modes (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void init(struct buf *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->data == NULL && p->n == 0)
{
  p->n = 0;
  p->data = NULL;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct buf s = { NULL, 1 };
  init(&s);
  return s.n;
}
