// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit S6): two owning locals, each freed on its path
// Pre-registered from record 0052 item 3 (audit round 1, S12): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int two(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return -1;
  }
  CERASTES_OWNING int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    free(a);
    return -1;
  }
  free(a);
  free(b);
  return 0;
}
