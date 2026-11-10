// expect: error leak @ L37
// mode: c23, c++20
// The block of an owning field must be freed before the struct local ends.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

void buf_init(struct buf *b CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  b->data = NULL;
  b->len = 0;
}

void buf_put(struct buf *b, int v)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  free(b->data);
  b->data = (int *)malloc(sizeof(int));
  if (b->data != NULL) {
    *b->data = v;
  }
}

int main(void)
{
  struct buf s;
  buf_init(&s);
  buf_put(&s, 1);
  return 0;
}
// end::source[]
