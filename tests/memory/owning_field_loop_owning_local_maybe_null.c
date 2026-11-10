// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 3 (limit 5): an owning local struct pointer that may be NULL (calloc is not tested), freed with its field and set to NULL inside the loop and freed again after it where non-null: the loop's value set keeps an owner that entered NULL as NULL, so the contents flag of its pointee survives (S41 (0) run 5b: the same fact written by hand verifies)
// Pre-registered from record 0056 item 3 (not yet run): verifies in both modes; it is the former pin owning_field_loop_owning_local_maybe_null_false_alarm.c (uninitialized @ L24 and L31 on the base) with a reworded header
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
