// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function that borrows a structure and promises to keep its buffer: the caller keeps a pointer into the buffer across the call.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void bump(struct buf *p CERASTES_OWNING CERASTES_NOT_INVALIDATING)
{
  if (p != NULL) {
    p->n = 0;
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b CERASTES_OWNING = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)calloc(1, sizeof(int));
  if (b->data == NULL) {
    free(b);
    return -1;
  }
  int *t = b->data;
  bump(b);
  *t = 1;
  free(b->data);
  free(b);
  return 0;
}
// end::source[]
