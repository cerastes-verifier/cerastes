// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: a by-value parameter's copy holds the block its site gives; every return frees it first
// Pre-registered from audit round 2 on Parts B/C (probes r3/): param_ret.c with the free before the early return: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int take(struct buf s)
    CERASTES_REQUIRES(s.data == NULL || cerastes_owns_block(s.data))
{
  if (s.n > 0) {
    free(s.data);
    return 1;
  }
  free(s.data);
  return 0;
}
