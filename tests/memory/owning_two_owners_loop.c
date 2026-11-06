// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 Loops (audit 3 B1, l1_two_owners.c): two owning locals of one block (b = a) both used in a loop: each owner's carrier must not claim the token twice; the loop writes through both and the block is freed through b
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 B1: verifies in both modes once the fix lands (it exited 3)
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 0;
  CERASTES_OWNING int *b = a;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    *a = i;
    *b = i;
    i += 1;
  }
  free(b);
  return 0;
}
