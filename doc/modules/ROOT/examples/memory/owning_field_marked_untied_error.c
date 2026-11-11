// expect: error uninitialized @ L30
// mode: c23, c++20
// Without the equality the caller cannot know the buffer is the same one, so it may hold no value after the call.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

void buf_touch(struct buf *b)
    CERASTES_REQUIRES(cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*b))
{
  b->n = 0;
}

int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return 1;
  }
  *s.data = 7;
  buf_touch(&s);
  if (s.data != NULL) {
    int y = *s.data;
    *s.data = y;
  }
  free(s.data);
  return 0;
}
// end::source[]
