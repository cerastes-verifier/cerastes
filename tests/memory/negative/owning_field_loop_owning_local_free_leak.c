// expect: error leak @ L31
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owning_local_free.c
// Pins: 0055 item 7: free(v) without free(v->data) drops the block the field holds; reported at the loop body's closing brace (a best reading: the back-edge row, as owning_field_loop_drop.c)
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): record 0055 items 4, 7: leak at the loop body's closing brace; to be confirmed by the run (question 160)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(void)
{
  struct buf *v CERASTES_OWNING = (struct buf *)calloc(1, sizeof(struct buf));
  if (v == NULL) {
    return 0;
  }
  v->data = (int *)malloc(sizeof(int));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(i <= 2 || v == NULL)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2 && v != NULL) {
      free(v);
      v = NULL;
    }
    i = i + 1;
  }  // <- here
  return 0;
}
