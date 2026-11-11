// expect: verified
// expect: native-ok
// mode: c23, c++20
// A structure whose buffer is allocated first and written by a function: the member says its block may hold no value yet.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
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
