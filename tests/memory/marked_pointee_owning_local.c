// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit S5): an owning and marked automatic local pointer to a structure with an owning field: the marked site closes nothing, so the allocation holds no promise about the field; the program writes the field and frees the block
// Pre-registered from record 0058 item 4 (audit S5, question 202): verifies in both modes (edg-backend's s5_local.c)
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
  free(b);
  return 0;
}
