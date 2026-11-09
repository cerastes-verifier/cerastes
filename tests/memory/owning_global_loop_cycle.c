// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("Loops (as built)") and audit round 2 (Parts C and D), probe h1: a loop that frees the owning global's block and allocates a new one every iteration, without a written invariant: the global is a loop place carried by its current value
// Pre-registered from record 0054 item 5 and audit probe h1 (verifies today): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(g);
    g = (int *)malloc(sizeof(int));
    if (g != NULL) {
      *g = i;
    }
    i += 1;
  }
  return 0;
}
