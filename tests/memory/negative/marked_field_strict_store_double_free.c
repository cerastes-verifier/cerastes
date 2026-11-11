// expect: error invalid-free @ L24
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/marked_field_strict_store.c
// Pins: 0058 item 4: the default checks stay under --strict-init: the owning marked field's block is freed twice
// Pre-registered from record 0058 item 4: one invalid-free at the second free
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = (int *)malloc(sizeof(int));
  free(b->data);
  free(b->data);  // <- here
  free(b);
  return 0;
}
