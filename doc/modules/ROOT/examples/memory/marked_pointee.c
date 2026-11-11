// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer to a structure with an owning member may be marked ref_to_uninit; a global structure may stay as it ends.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void touch(struct buf *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->n = 0;
}

int main(void)
{
  touch(&g);
  return g.n;
}
// end::source[]
