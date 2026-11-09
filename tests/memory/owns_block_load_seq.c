// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (0047 item 9's sequencing registration for a site through a load) and audit round 2 item 2 (probe s1): the read of *a is sequenced before the consuming call (v = *a in its own statement)
// Pre-registered from the sequenced form of the audit probe s1 (derived: verifies in both modes)
#include <cerastes.h>
#include <stdlib.h>

int consume(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(r, cerastes_owns(*pp) && r == 0)
{
  free(*pp);
  return 0;
}

int use(void)
{
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 1;
  int v = *a;
  int r = v + consume(&a);
  return r;
}
