// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owning_field_replace.c
// Pins: 0055 item 4 (a dropped block through a pointer): the borrower overwrites p->data without freeing the block it holds; no row at the store (S40 (d)): the end-of-function row reports "`replace` may return while it still owns an allocation that its `ensures` does not hand back" at the closing line
// Pre-registered from record 0055 item 4 (S40 (d), no dropped-block row): leak at the closing brace of replace (the end-of-function row)
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
  p->data = (int *)malloc(sizeof(int));
  if (p->data != NULL) {
    *p->data = v;
  }
}  // <- here

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
