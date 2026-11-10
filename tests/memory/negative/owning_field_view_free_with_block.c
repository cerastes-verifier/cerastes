// expect: error leak @ L24
// mode: c23, c++20
// twin: tests/memory/owning_field_view.c
// Pins: 0055 items 3, 4: the field of the malloc'd structure is given a block that free(a) then drops: the end-of-function row reports "`from_malloc` may return while it still owns an allocation that its `ensures` does not hand back" at the closing line
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), N6 (item 3, "Allocation views close nothing"):  leak at the closing brace of from_malloc
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int from_malloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *a = (struct buf *)malloc(sizeof(struct buf));
  if (a == NULL) {
    return -1;
  }
  a->data = (int *)malloc(sizeof(int));
  free(a);
  return 0;
}  // <- here

int from_calloc(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *a = (struct buf *)calloc(1, sizeof(struct buf));
  if (a == NULL) {
    return -1;
  }
  free(a);
  return 0;
}
