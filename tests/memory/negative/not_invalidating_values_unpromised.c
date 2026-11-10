// expect: error check @ L41
// mode: c23, c++20
// twin: tests/memory/not_invalidating_values_tied.c
// Pins: 0058 item 2 ("It names pointers, never values"): without the written p->n == old(p->n) the marked contract says nothing about b->n, so the caller's check may not hold: the marker is no promise about values
// Pre-registered from record 0058 item 2: one check error at the check
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  if (p->data != NULL) {
    *p->data = 0;
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
  b->n = 5;
  touch(b);
  CERASTES_CHECK(b->n == 5);  // <- here
  buf_destroy(b);
  return 0;
}
