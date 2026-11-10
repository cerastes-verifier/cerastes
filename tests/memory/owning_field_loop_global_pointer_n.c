// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 7 (Part D): a loop over an owning struct buf * global that touches only the non-owning field gp->n; main ends holding the global (exempt)
// Pre-registered from the il-analyst's Part D probes (il/p r1, r2, r3, lg6, lg): lg6.c: verifies in both modes
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
  CERASTES_DECREASES(5 - i)
  while (i < 5) {
    gp->n = i;
    i += 1;
  }
  return 0;
}
