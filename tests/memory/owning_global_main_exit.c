// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (question 174, P4417R0 (2.3)): main returns while the owning global holds a block: exempt (an owner with static storage need not delete its object)
// Pre-registered from record 0054 item 5 ("main"; S39 b1): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void fill(void)
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
  fill();
  return 0;
}
