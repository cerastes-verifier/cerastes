// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit B2): an owning local that is NULL at loop entry and allocates every iteration, freeing the block of the previous one first: free(NULL) the first time; the carried held ghost makes each reassignment row hold
// Pre-registered from record 0052 item 3 (audit round 1, S12): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(v);
    v = (int *)malloc(sizeof(int));
    i += 1;
  }
  free(v);
  return 0;
}
