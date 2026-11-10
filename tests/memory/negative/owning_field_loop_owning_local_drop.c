// expect: error leak @ L30
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owning_local.c
// Pins: 0055 items 4, 7 (S5): the loop replaces v->data without freeing the block it holds; no row at a store through a pointer, so the loop's back-edge row reports it at the loop body's closing brace
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), leak at the loop body's closing brace (as owning_field_loop_drop.c; the exact row for an owning local's field is a best reading)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int run(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING struct buf *v = (struct buf *)calloc(1, sizeof(struct buf));
  if (v == NULL) {
    return -1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    v->data = (int *)malloc(sizeof(int));
    if (v->data != NULL) {
      *v->data = i;
    }
    i += 1;
  }  // <- here
  free(v->data);
  free(v);
  return 0;
}
