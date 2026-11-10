// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/not_invalidating_borrower.c
// Pins: 0058 item 2 ("The check on the contract"): a destroyer marked not_invalidating: requires owns_block(p) with no ensures site on p is a block taken and not returned: unsupported_annotation at the parameter ("the contract of buf_destroy takes owns_block(p) without returning it")
// Pre-registered from record 0058 item 2: rejected unsupported_annotation at the parameter
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void buf_destroy(struct buf *p CERASTES_NOT_INVALIDATING)  // <- here
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
  buf_destroy(b);
  return 0;
}
