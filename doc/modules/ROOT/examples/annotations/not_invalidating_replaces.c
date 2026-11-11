// expect: error postcondition @ L22
// mode: c23, c++20
// A body that frees the buffer and allocates another changes the pointer, which not_invalidating forbids.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_OWNING CERASTES_NOT_INVALIDATING)
{
  if (p != NULL) {
    free(p->data);
    p->data = (int *)malloc(sizeof(int));
    if (p->data != NULL) {
      *p->data = 0;
    }
  }
}
// end::source[]
