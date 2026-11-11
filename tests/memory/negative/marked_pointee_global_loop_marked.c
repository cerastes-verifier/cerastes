// expect: error invalid-free @ L25
// expect: error invalid-free @ L25
// expect: error invalid-free @ L25
// expect: error leak @ L28
// expect: error permission @ L28
// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/marked_pointee_global_loop_plain.c
// Pins: 0058 item 4 (audit S5): the same loop with the global marked ref_to_uninit: the marked site closes nothing, so the loop does not own gp->x: the free is three invalid-free rows and the loop's end rows are a leak and two permission errors (the sound direction; a VIR case cannot carry this twin, il-analyst report 6)
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): three invalid-free at the free and one leak and two permission errors at the loop (partF3/gp_marked_replaces.c)
#include <cerastes.h>
#include <stdlib.h>
struct leaf { int *x CERASTES_OWNING; int k; };
struct leaf *gp CERASTES_OWNING CERASTES_REF_TO_UNINIT;
int main(void)
{
  gp = (struct leaf *)calloc(1, sizeof(struct leaf));
  if (gp == NULL) {
    return 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(gp->x);  // <- here
    gp->x = (int *)malloc(sizeof(int));
    i++;
  }  // <- here
  return 0;
}
