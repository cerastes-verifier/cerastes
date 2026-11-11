// expect: verified
// expect: native-ok
// mode: c23, c++20
// With must_init the callee promises a value, so the caller knows what the owning member holds.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void init(struct buf *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->data = NULL;
  p->n = 0;
}

int main(void)
{
  struct buf s CERASTES_UNINIT;
  init(&s);
  free(s.data);
  return s.n;
}
// end::source[]
