// expect: error leak @ L24
// mode: c23, c++20
// After the call a local structure may hold a block in its owning member, so its end is reported.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 0;
}

int main(void)
{
  struct buf s = { NULL, 0 };
  touch(&s);
  return s.n;
}
// end::source[]
