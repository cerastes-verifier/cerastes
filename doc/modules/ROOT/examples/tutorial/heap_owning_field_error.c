// expect: error leak @ L33
// mode: c23, c++20
// The destructor frees the structure but forgets the buffer its owning field holds.
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
