// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("the leaf itself owns(p->q)"): the atom covers only the field's cell, with no covering owns(*p), and still closes with the clause; the caller lends the cell out of a structure it owns whole and gets it back
// Pre-registered from record 0055 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void renew(struct buf *p, int v)
    CERASTES_REQUIRES(cerastes_owns(p->data))
    CERASTES_ENSURES(cerastes_owns(p->data))
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
  renew(b, 1);
  buf_destroy(b);
  return 0;
}
