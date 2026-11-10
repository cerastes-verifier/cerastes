// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 4: a borrower with owns(*p) frees the block its owning field holds, stores a new one and gives it back (the closure clause is in the postcondition too); its caller destroys the structure afterwards
// Pre-registered from record 0055 items 3, 4: verifies in both modes (written by hand, the clause verifies on the d2b2a encoder)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void replace(struct buf *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  free(p->data);
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = v;
  }
}

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
  replace(b, 3);
  buf_destroy(b);
  return 0;
}
