// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3: a borrower with owns(*p) holds the field's block as well (the closure clause), so it reads *p->data after a null test and says in its postcondition what it read; its caller allocates the block, writes 9 and passes the structure on
// Pre-registered from record 0055 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int first_or(struct buf *p, int dflt)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && (p->data == NULL || r == *p->data))
{
  if (p->data == NULL) {
    return dflt;
  }
  return *p->data;
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
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)malloc(sizeof(int));
  if (b->data == NULL) {
    buf_destroy(b);
    return -1;
  }
  *b->data = 9;
  int v = first_or(b, 5);
  buf_destroy(b);
  return v;
}
