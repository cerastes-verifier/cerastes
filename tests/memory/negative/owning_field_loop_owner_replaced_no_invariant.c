// expect: error invalid-free @ L30
// expect: error invalid-free @ L30
// expect: error invalid-free @ L30
// expect: error uninitialized @ L30
// expect: error lifetime @ L32
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owner_replaced_written.c
// Pins: 0056 item 8 (question 201; not a false alarm): a loop that frees the owning local struct pointer and replaces it by a fresh malloc, freeing the field only in its first iteration, needs the written, guarded site CERASTES_INVARIANT(v == NULL || i != 0 || v->data == NULL || cerastes_owns_block(v->data)): a fact true in the first iteration only can only be written (S41 (0) run 6: with it the loop verifies); without it no closure is carried across a replacement of the owner, so the free of v->data and the read after the replacement are reported. The expectations are the errors the encoder reports today, kept by the owner's answer 201
// Pre-registered from the final re-audit (r5/s1_realloc_v2.c), observed on the session build after the last fix: the expect lines are the actual outcome; M4.5d2b2c keeps them (question 201) and names the invariant the loop needs
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
