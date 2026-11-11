// expect: verified
// expect: native-ok
// mode: c23, c++20
// A declaration marked not_invalidating needs no tie for its owning field: the marker says the field is unchanged.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)calloc(1, sizeof(int));
  if (b->data == NULL) {
    buf_destroy(b);
    return -1;
  }
  int *t = b->data;
  touch(b);
  *t = 1;
  buf_destroy(b);
  return 0;
}
// end::source[]
