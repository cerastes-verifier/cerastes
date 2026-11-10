// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 8 (question 201): the alias loop of owning_field_loop_alias_no_invariant.c with the written invariant owns(*p): the loop through the non-owning alias v = p of a borrowed structure frees and replaces v->data and gives the block back
// Pre-registered from S41 (0) run 1a on the frozen base (this program was run there before it was a test): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void churn(struct buf *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 5)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  struct buf *v = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(v->data);
    v->data = (int *)malloc(sizeof(int));
    if (v->data != NULL) {
      *v->data = i;
    }
    i += 1;
  }
}
