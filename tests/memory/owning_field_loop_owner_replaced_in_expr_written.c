// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 8 (question 201): the replaced-owner loop with the replacement inside an expression, with the guarded site written in its invariant
// Pre-registered from record 0056 item 8 (not yet run): verifies in both modes (the same site as S41 (0) run 6, whose statement form verifies)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int f(void)
{
  struct buf *v CERASTES_OWNING = (struct buf *)calloc(1, sizeof(struct buf));
  if (v == NULL) {
    return 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(v == NULL || i != 0 || v->data == NULL || cerastes_owns_block(v->data))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (v != NULL && i == 0) {
      free(v->data);
    }
    if (v != NULL) {
      free(v);
    }
    int k = use((v = (struct buf *)malloc(sizeof(struct buf))) != NULL);
    if (v != NULL) {
      v->n = i + k - k;
    }
    i = i + 1;
  }
  if (v != NULL) {
    free(v);
  }
  return 0;
}
