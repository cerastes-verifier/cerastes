// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3, 0055 items 3, 7 (S5): an owning local struct buf * v and a loop that frees and replaces the block of v->data each iteration: v is a loop place keyed by its current value and carries the closure of its field
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes
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
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(v->data);
    v->data = (int *)malloc(sizeof(int));
    if (v->data != NULL) {
      *v->data = i;
    }
    i += 1;
  }
  free(v->data);
  free(v);
  return 0;
}
