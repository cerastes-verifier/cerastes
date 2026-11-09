// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 and audit round 2 item 2 (probe s3): the sequenced form for a block site on a global: *g is read in its own statement before the consuming call
// Pre-registered from the sequenced form of the audit probe s3 (derived: verifies in both modes)
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

int consume(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns_block(g))
    CERASTES_ENSURES(r, cerastes_owns(g) && r == 0)
{
  free(g);
  return 0;
}

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    return 1;
  }
  *g = 1;
  int v = *g;
  int r = v + consume();
  return r;
}
