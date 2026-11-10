// expect: verified
// expect: native-ok
// mode: c23, c++20
// A structure that owns a buffer is allocated, filled and destroyed: destroying it frees the buffer and then the structure.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

CERASTES_OWNING struct buf *buf_make(int v)
{
  struct buf *b = (struct buf *)malloc(sizeof(struct buf));
  if (b == NULL) {
    return NULL;
  }
  b->len = 1;
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = v;
  }
  return b;
}

void buf_destroy(struct buf *b)
    CERASTES_REQUIRES(b == NULL || cerastes_owns_block(b))
{
  if (b != NULL) {
    free(b->data);
    free(b);
  }
}

int main(void)
{
  struct buf *b = buf_make(7);
  buf_destroy(b);
  return 0;
}
// end::source[]
