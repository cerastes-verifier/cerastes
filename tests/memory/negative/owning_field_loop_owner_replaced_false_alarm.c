// expect: error invalid-free @ L30
// expect: error invalid-free @ L30
// expect: error invalid-free @ L30
// expect: error uninitialized @ L30
// expect: error lifetime @ L32
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owning_local_free.c
// Pins: 0055 item 7 (a false alarm pinned; the program is correct): an owning local struct pointer freed and replaced by a fresh malloc in the loop: no closure is carried across a replacement of the owner, so the loop's carry of v loses its pointee's owning field (the free of v->data and the read after the replacement are reported); a loop that only frees and nulls the owner (owning_field_loop_owning_local_free.c) verifies. The expectations are the errors the encoder reports today
// Pre-registered from the final re-audit (r5/s1_realloc_v2.c), observed on the session build after the last fix: the expect lines are the actual outcome, recorded as a pin
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
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(i == 0 || v == NULL || v->n == i - 1)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (v != NULL && i == 0) {
      free(v->data);
    }
    if (v != NULL) {
      free(v);
    }
    v = (struct buf *)malloc(sizeof(struct buf));
    if (v != NULL) {
      v->n = i;
    }
    i = i + 1;
  }
  if (v != NULL) {
    free(v);
  }
  return 0;
}
