// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3: the block moves from an inner owning local to an owner that outlives the block (keeper = t): the inner local's lifetime-end row finds an owner that remains holding the same value, and the outer one is freed
// Pre-registered from record 0052 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int handoff(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *keeper = NULL;
  {
    CERASTES_OWNING int *t = (int *)malloc(sizeof(int));
    if (t == NULL) {
      return -1;
    }
    *t = 3;
    keeper = t;
  }
  free(keeper);
  return 0;
}
