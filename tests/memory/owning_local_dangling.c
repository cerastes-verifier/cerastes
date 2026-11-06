// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit S5): a = malloc; free(a); k = malloc: the second allocation may return a's block: a is dangling (held_a cleared by the free), so a's ending row does not see the block that k holds
// Pre-registered from record 0052 item 3 (audit round 1, S12): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int again(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return -1;
  }
  *a = 1;
  free(a);
  CERASTES_OWNING int *k = (int *)malloc(sizeof(int));
  if (k == NULL) {
    return -1;
  }
  *k = 2;
  free(k);
  return 0;
}
