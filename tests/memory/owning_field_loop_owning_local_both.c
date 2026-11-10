// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 7: the same loop reading v->n and writing through *v->data
// Pre-registered from the il-analyst's Part D probes (il/p r1, r2, r3, lg6, lg): r3.c: verifies in both modes
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
  v->data = (int *)malloc(sizeof(int));
  if (v->data != NULL) {
    *v->data = 3;
  }
  int i = 0;
  int t = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    v->n = i;
    if (v->data != NULL) {
      *v->data = i;
    }
    i += 1;
  }
  free(v->data);
  free(v);
  return t;
}
