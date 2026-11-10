// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3, 0055 item 5 (audit S2): an owning local hands its block to a struct local's owning leaf (s.data = v; v = NULL;): the leaf is the owner afterwards and frees it
// Pre-registered from audit round 2 on Parts B/C (probes r3/): v_to_leaf.c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(void)
{
  int *v CERASTES_OWNING = (int *)malloc(sizeof(int));
  if (v != NULL) {
    *v = 1;
  }
  struct buf s = { NULL, 0 };
  s.data = v;
  v = NULL;
  free(s.data);
  s.data = NULL;
  return 0;
}
