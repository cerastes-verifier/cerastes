// expect: error uninitialized @ L24
// expect: error uninitialized @ L31
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_owning_local_free.c
// Pins: 0055 item 7 (a false alarm pinned): an owning local struct pointer that may be NULL (calloc is not tested), freed with its field and set to NULL inside the loop and freed again after it where non-null: the loop's carry of the owning local loses the contents flag of its pointee on the path where it may be NULL; the same loop after a NULL test (owning_field_loop_owning_local_free.c) verifies. The expectations are the errors the encoder reports today
// Pre-registered from the final re-audit (probes r4/ and r5/), observed on the session build after the fixes: d_pf4.c: the expect lines are the actual outcome, recorded as a pin
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
    free(v->data);
    free(v);
  }
  return 0;
}
