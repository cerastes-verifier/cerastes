// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit S5): a loop that frees and replaces gp->x of an owning global pointer to a structure with an owning field, the global NOT marked: the loop carries the global's closure and verifies (the twin marks the global)
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): verifies in both modes (build/m4.5d2b2d/partF3/gp_unmarked_replaces.c)
#include <cerastes.h>
#include <stdlib.h>
struct leaf { int *x CERASTES_OWNING; int k; };
struct leaf *gp CERASTES_OWNING;
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
    free(gp->x);
    gp->x = (int *)malloc(sizeof(int));
    i++;
  }
  return 0;
}
