// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (audit B2): the way out of owning_loop_free_not_reset.c: the last iteration frees the owner and assigns NULL, and the invariant says k > 0 || v == NULL, so after the loop (k == 0) the carried held_v ==> v != null is false and the ending row at the return holds; n is at least 1 so that the invariant holds at entry
// Pre-registered from record 0052 item 3 (audit round 1, S12): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int last_frees(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 1;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n && (k > 0 || v == NULL))
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    if (k == 0) {
      free(v);
      v = NULL;
    }
  }
  return 0;
}
