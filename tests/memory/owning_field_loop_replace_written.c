// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 7: the user writes the closure clause beside owns(*p) in the invariant, in exactly the closure's shape: it is merged (the written clause takes the closure's place, 0054 item 5), so the program verifies as the one without it
// Pre-registered from record 0055 item 7 (loops): verifies in both modes (the invariant is S40 (g) g1_replace.c's, verbatim)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void churn(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(cerastes_owns(*p) && (p->data == NULL || cerastes_owns_block(p->data)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(p->data);
    p->data = (int *)malloc(sizeof(int));
    if (p->data != NULL) {
      *p->data = i;
    }
    i += 1;
  }
}
