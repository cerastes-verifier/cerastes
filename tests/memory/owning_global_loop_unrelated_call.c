// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("Loops (as built)") and audit round 2 (Parts C and D), probe a1: main holds a block through the owning global and the loop calls a function that names no global: the call must not make held_g unknown
// Pre-registered from record 0054 item 5 (a call whose contract does not name g leaves held_g as it was): verifies in both modes (audit probe a1)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

int sq(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x * x)
{
  return x * x;
}

int main(void)
{
  g = (int *)malloc(sizeof(int));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    (void)sq(i);
    i += 1;
  }
  free(g);
  g = NULL;
  return 0;
}
