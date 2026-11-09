// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (the exemption of main's leak row, one term per owning global, nested left) and audit round 2 (Parts C and D), probe i1: three owning globals all holding a block when main returns: three exemption terms (the lint flattens the nested disjunction)
// Pre-registered from audit probe i1 (verifies when written): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *a = NULL;
CERASTES_OWNING static int *b = NULL;
CERASTES_OWNING static int *c = NULL;

int main(void)
{
  a = (int *)malloc(sizeof(int));
  b = (int *)malloc(sizeof(int));
  c = (int *)malloc(sizeof(int));
  return a == b || b == c;
}
