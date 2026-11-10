// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 5, 7 (re-audit B2): a struct local's leaf replaced in a loop and freed after it
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): d_ok.c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(void)
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
  free(s.data);
  return 0;
}
