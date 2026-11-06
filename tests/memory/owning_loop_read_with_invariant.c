// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 Loops (audit 3 S1, l15_with_inv.c): the same loop with the value kept by a user invariant over the owner (v != NULL && *v == 5), the workaround that already verifies
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 S1: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 5;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && s == 5 * i && v != NULL && *v == 5)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += *v;
    i += 1;
  }
  free(v);
  return s;
}
