// expect: verified
// expect: native-ok
// mode: c23, c++20
// A copy of a structure hands the owning field's block to the copy: the source is set to NULL and the copy frees the block.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return 1;
  }
  *s.data = 1;
  struct buf t = s;
  s.data = NULL;
  free(t.data);
  return 0;
}
// end::source[]
