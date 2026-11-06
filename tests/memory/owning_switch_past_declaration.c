// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit S4): a case label that jumps past the declaration of an owning local, which has no initializer (cerastes rejects a goto over any declaration, and a label over one with an initializer): held_p is false at method entry and at the declaration, so the ending row at the end of the switch reads no havocked p and reports no leak on the default path
// Pre-registered from record 0052 item 3 (audit round 1, S12): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int sw(int c)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  switch (c) {
    CERASTES_OWNING int *p;
  case 0:
    p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      return -1;
    }
    *p = 1;
    free(p);
    break;
  default:
    break;
  }
  return 0;
}
