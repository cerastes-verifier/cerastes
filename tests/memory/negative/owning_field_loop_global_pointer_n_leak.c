// expect: error leak @ L34
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global_pointer_n.c
// Pins: 0055 items 5, 7: a plain allocation main still holds at its end is not the owning global's block: the end-of-function row at the closing line
// Pre-registered from the il-analyst's Part D probes (il/p r1, r2, r3, lg6, lg): record 0047 item 7: leak at main's closing brace
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
  int *w = (int *)malloc(sizeof(int));
  if (w != NULL) {
    *w = 1;
  }
  return 0;
}  // <- here
