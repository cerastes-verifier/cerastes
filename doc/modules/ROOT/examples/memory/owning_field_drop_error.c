// expect: error leak @ L19
// mode: c23, c++20
// A function that overwrites the owning field must free the block it holds first.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

void buf_clear(struct buf *b)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  b->data = NULL;
  b->len = 0;
}
// end::source[]
