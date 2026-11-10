// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/memory/not_invalidating_borrower.c
// Pins: 0058 item 2 (the same revision): the body allocates into p->data where it was NULL on entry, so the field changes from NULL to a block; the caller of this twin never holds t, the caller's field is calloc'd NULL and the caller reports nothing; the end of touch reports the implicit ensures
// Pre-registered from record 0058 item 2 (as revised): one postcondition error at the closing brace of touch (observed with the tie written: build/m4.5d2b2d/emul/j2_emul.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_OWNING CERASTES_NOT_INVALIDATING)
{
  if (p != NULL && p->data == NULL) {
    p->data = (int *)malloc(sizeof(int));
    if (p->data != NULL) {
      *p->data = 0;
    }
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
  touch(b);
  buf_destroy(b);
  return 0;
}
