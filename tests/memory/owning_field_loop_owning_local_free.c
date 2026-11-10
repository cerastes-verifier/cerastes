// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3, 0055 item 7 (re-audit S1): the same with a block stored into v->data first: the loop frees the field's block and the structure and sets v to NULL
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): d_pf3.c with a stored block: verifies in both modes
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
      free(v->data);
      free(v);
      v = NULL;
    }
    i = i + 1;
  }
  return 0;
}
