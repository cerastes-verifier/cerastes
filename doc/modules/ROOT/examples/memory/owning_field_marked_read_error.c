// expect: error uninitialized @ L18
// mode: c23, c++20
// A marked member's buffer may hold no value, so a function that reads it first is reported.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

void buf_sum(struct buf *b)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  if (b->data != NULL) {
    b->n = *b->data;
  }
}

int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  buf_sum(&s);
  free(s.data);
  return 0;
}
// end::source[]
