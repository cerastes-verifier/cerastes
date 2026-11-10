// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owning_field_copy_over.c
// Pins: 0055 items 5, 6: the whole-struct store t = s; drops the block t.data holds: "assigning to owning field `t.data` drops the block it holds", at the copy (copy_over.c)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): copy_over.c: leak at the copy
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
  t = s;  // <- here
  return t.n;
}
