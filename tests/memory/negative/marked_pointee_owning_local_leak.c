// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/marked_pointee_owning_local.c
// Pins: 0058 item 4 (audit S5): the same program without the free: the owner is dropped, the default mode's leak
// Pre-registered from record 0058 item 4 (audit S5, question 202): one leak at the return
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING CERASTES_REF_TO_UNINIT struct buf *b = (struct buf *)malloc(sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = NULL;
  b->n = 0;
  return 0;  // <- here
}
