// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit S5): an owning and marked static local pointer to a structure with an owning field is accepted and closes nothing
// Pre-registered from record 0058 item 4 (audit S5, question 202): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int next(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING CERASTES_REF_TO_UNINIT static struct buf *cache = NULL;
  if (cache == NULL) {
    cache = (struct buf *)malloc(sizeof(struct buf));
    if (cache == NULL) {
      return -1;
    }
    cache->data = NULL;
  }
  cache->n = 1;
  return 0;
}
