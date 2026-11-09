// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("Loops (as built)") and audit round 2 (Parts C and D), probe h11: init() with owns(g) in both lists (frees the old block, allocates a new one) called in a loop: each call gives the loop back a global that is null or holds a block
// Pre-registered from record 0054 item 5 (audit probe h11; an uninitialized false alarm at the call when written): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void init(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (int *)malloc(sizeof(int));
  if (g != NULL) {
    *g = 1;
  }
}

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    init();
    i += 1;
  }
  return 0;
}
