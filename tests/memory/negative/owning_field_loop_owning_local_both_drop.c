// expect: error leak @ L36
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owning_local_both.c
// Pins: 0055 item 7: the loop also sets v->data to NULL without freeing it, dropping the block the closure carries: the back-edge row at the loop body's closing brace
// Pre-registered from record 0055 items 4, 7: leak at the loop body's closing brace (as owning_field_loop_owning_local_n_drop.c)
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
  v->data = (int *)malloc(sizeof(int));
  if (v->data != NULL) {
    *v->data = 3;
  }
  int i = 0;
  int t = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    v->n = i;
    if (v->data != NULL) {
      *v->data = i;
    }
    v->data = NULL;
    i += 1;
  }  // <- here
  free(v->data);
  free(v);
  return t;
}
