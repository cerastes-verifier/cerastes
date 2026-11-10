// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 5, 7 (re-audit B2): an owning function returns the block of a struct local's owning leaf after a loop that frees and replaces it: the returned owner is exempt at the return
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): d_ret.c: verifies in both modes (it exited 3 before the fix)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING int *f(void)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(s.data);
    s.data = (int *)malloc(sizeof(int));
    if (s.data != NULL) {
      *s.data = i;
    }
    i = i + 1;
  }
  return s.data;
}
