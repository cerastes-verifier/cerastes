// expect: error uninitialized @ L26
// mode: c23, c++20
// Without the marker on the member, passing a buffer that has no value to buf_load is reported.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_load(struct buf *b)
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
  buf_load(&s);
  free(s.data);
  return 0;
}
// end::source[]
