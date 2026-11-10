// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_param_return.c
// Pins: 0055 item 6: the early return leaves with the copy still holding the block, at the return (param_ret.c)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): param_ret.c: leak at the early return
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
    return 1;  // <- here
  }
  free(s.data);
  return 0;
}
