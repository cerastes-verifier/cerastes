// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 8 (question 201): the replaced-owner loop of owning_field_loop_owner_replaced_no_invariant.c with the guarded site written in its invariant: an owning local struct pointer is freed (its field only in the first iteration) and replaced by a fresh malloc in every iteration
// Pre-registered from S41 (0) run 6 on the frozen base (this program was run there before it was a test): verifies in both modes
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
  CERASTES_INVARIANT(v == NULL || i != 0 || v->data == NULL || cerastes_owns_block(v->data))
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
