// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_owner_to_leaf.c
// Pins: 0055 item 5: the leaf received the block and nothing frees it, at the return where s ends
// Pre-registered from audit round 2 on Parts B/C (probes r3/): record 0055 item 5: leak at the return statement
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
  return s.n;  // <- here
}
