// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// A function that frees what it is given invalidates every pointer into it: the marker is rejected on it.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_destroy(struct buf *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
  free(p->data);
  free(p);
}
// end::source[]
