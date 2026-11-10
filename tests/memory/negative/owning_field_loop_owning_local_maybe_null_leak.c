// expect: error leak @ L33
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owning_local_maybe_null.c
// Pins: 0056 item 3 (limit 5): the same function without the free(v->data) after the loop: the structure is freed with its field's block still owned on the path where the loop left it, so the block is dropped
// Pre-registered from record 0056 item 3 (not yet run): leak at the return of f (the end-of-function row names the dropped block, record 0055 item 4); corrected after the first run by question 160 from L32 (the return) to L33, the closing brace of f, where the end-of-function row reports it
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
{
  struct buf *v CERASTES_OWNING = (struct buf *)calloc(1, sizeof(struct buf));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (v != NULL) {
      free(v->data);
      free(v);
      v = NULL;
    }
    i = i + 1;
  }
  if (v != NULL) {
    free(v);
  }
  return 0;
}
