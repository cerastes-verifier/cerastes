// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 6 (limit 3): the same loop with the written invariant gp == NULL || owns_block(gp): the site the user writes beside the carried closure (S41 (0) run 3 on the base: rejected as an invariant that reads gp->data, which the loop did not hold)
// Pre-registered from record 0056 item 6 (not yet run on the L-2 build): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING static struct buf *gp = NULL;

int main(void)
{
  gp = (struct buf *)calloc(1, sizeof(struct buf));
  if (gp == NULL) {
    return 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 5)
  CERASTES_INVARIANT(gp == NULL || cerastes_owns_block(gp))
  CERASTES_DECREASES(5 - i)
  while (i < 5) {
    free(gp->data);
    gp->data = (int *)malloc(sizeof(int));
    if (gp->data != NULL) {
      *gp->data = i;
    }
    i += 1;
  }
  return 0;
}
