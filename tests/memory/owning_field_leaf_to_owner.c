// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3, 0055 item 5 (audit S2): a struct local's leaf hands its block to an owning local (v = s.data; s.data = NULL;): the owning local is the owner afterwards and frees it
// Pre-registered from audit round 2 on Parts B/C (probes r3/): leaf_to_v.c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 1;
  }
  int *v CERASTES_OWNING = s.data;
  s.data = NULL;
  free(v);
  v = NULL;
  return 0;
}
