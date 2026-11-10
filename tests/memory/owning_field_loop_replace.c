// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 3, 7: a loop that frees the owning field's block and takes a new one every iteration, under the written invariant owns(*p) alone: the closure clause p->data == NULL || owns_block(p->data) is added to the invariant (S40 (g) g1_replace.c has it written)
// Pre-registered from record 0055 item 7 (loops): verifies in both modes (S40 (g) g1_replace.c on the frozen base, clause by hand: verified, 6.3 s)
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
  CERASTES_INVARIANT(cerastes_owns(*p))
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
