// expect: error permission @ L19
// mode: c23, c++20
// An owning field may receive only a block the function holds, not the address of a local.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

void buf_put(struct buf *b, int v)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  int x = v;
  free(b->data);
  b->data = &x;
}
// end::source[]
