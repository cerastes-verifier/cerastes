// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_field_leaf_to_owner.c
// Pins: 0055 item 5: the owning local received the block and nothing frees it, at the return where v ends
// Pre-registered from audit round 2 on Parts B/C (probes r3/): record 0052 item 3: leak at the return statement
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
  return v == NULL;  // <- here
}
