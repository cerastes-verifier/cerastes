// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit round 2 N5, f_for_init.c): an owning local declared in a for-init, read and freed in the body, then left by a break: the loop reads v (it was a false lifetime), and the scope exit at the break finds the block freed
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) N5: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int n = 1;
  CERASTES_INVARIANT(0 <= n && n <= 1)
  CERASTES_DECREASES(n)
  for (CERASTES_OWNING int *v = (int *)malloc(sizeof(*v)); n > 0; n--) {
    if (v != NULL) {
      *v = 1;
      free(v);
    }
    break;
  }
  return 0;
}
