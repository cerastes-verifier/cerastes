// expect: verified
// expect: native-ok
// mode: c23, c++20
// A marked owning member: a buffer that is allocated and not yet written can be handed to a function that writes it.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
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
