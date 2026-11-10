// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global_pointer.c
// Pins: 0056 items 6 and 7, audit round 2 B1: main frees the owning global gp before a loop that frees and replaces gp->data: the entry row's mapped permission (the invariant needs to own the whole allocation gp points to), and never the unmapped if.failed of the entry splits read through gp's freed block
// Pre-registered from audit round 2 B1 (probe p2_gp_freed_before.c): the base's mapped error, permission at the `while` line, in both modes, exit 1; Part L-2 reports an unmapped exit 3 until question 211's fix; not yet run on the build with it [Corrected after the first run on the build with 208-211, 213 (question 160): in c++20 an error of the loop's entry row is reported at the first attribute line of the loop, in c23 at the `while`, so the attributes and the `while` now stand on one line (every later line moved up by two).]
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
  free(gp);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 5) CERASTES_DECREASES(5 - i) while (i < 5) {
    free(gp->data);
    gp->data = (int *)malloc(sizeof(int));
    if (gp->data != NULL) {
      *gp->data = i;
    }
    i += 1;
  }
  gp = NULL;
  return 0;
}
