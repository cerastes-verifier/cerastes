// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: calloc leaves the owning field NULL, so removing free(v->data) alone is not a leak; the must-fail form is owning_field_loop_owning_local_free_leak.c on the program that stores a block into the field first
// Pins: 0052 item 3, 0055 item 7 (re-audit S1): an owning local struct pointer assigned in the loop: freed with its field and set to NULL; the calloc'd structure's field is NULL
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): d_pf3.c: verifies in both modes
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
  CERASTES_INVARIANT(i <= 2 || v == NULL)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2 && v != NULL) {
      free(v->data);
      free(v);
      v = NULL;
    }
    i = i + 1;
  }
  return 0;
}
