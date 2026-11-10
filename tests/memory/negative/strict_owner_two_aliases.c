// expect: error invalid-free @ L34
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 ("accepted although wrong"): a member released through two aliases of its structure: b->data and q->data (q == b) are each an owner by name for p6, and the default mode's invalid-free reports the second release
// Pre-registered from record 0058 item 3: one invalid-free at the second free under --strict-owner
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

int f(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = make();
  struct buf *q = b;
  free(b->data);
  free(q->data);  // <- here
  free(b);
  return 0;
}
