// expect: error uninitialized @ L26
// mode: c23, c++20
// The same program without the marker: the buffer has no value at the call, and buf_fill needs one.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_fill(struct buf *b)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  if (b->data != NULL) {
    *b->data = 1;
  }
}

int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  buf_fill(&s);
  free(s.data);
  return 0;
}
// end::source[]
