// expect: rejected cerastes-unsupported_annotation @ L19
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: malloc's result goes to a member that is not marked.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void fill(struct buf *b)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  free(b->data);
  b->data = (int *)malloc(sizeof(int));
}
// end::source[]
