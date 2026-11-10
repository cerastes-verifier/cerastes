// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 5, 6: t = s; stores to the owning leaf t.data, so the reassignment row runs over the block t holds: here it was freed first
// Pre-registered from audit round 2 on Parts B/C (probes r3/): copy_over_ok.c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(void)
{
  struct buf s = { NULL, 0 };
  struct buf t = { NULL, 0 };
  t.data = (int *)malloc(sizeof(int));
  if (t.data != NULL) {
    *t.data = 1;
  }
  free(t.data);
  t = s;
  return t.n;
}
