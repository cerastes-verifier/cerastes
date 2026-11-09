// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (P75 (a): EDG unions the attribute lists of the declarations, the encoder compares every declaration): an owning global with an extern declaration and a definition that BOTH carry owning is accepted (only a declaration without the attribute beside one with it is annotation_mismatch); a function names owns(g), main assigns g first (external linkage: unknown value in C++)
// Pre-registered from record 0054 item 5 and orchestrator note (the encoder wrongly refused it when written): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING extern int *g;
CERASTES_OWNING int *g = NULL;

void reset(void)
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
  g = NULL;
  reset();
  return 0;
}
