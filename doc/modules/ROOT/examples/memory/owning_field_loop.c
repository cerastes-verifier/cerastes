// expect: verified
// expect: native-ok
// mode: c23, c++20
// A loop that replaces the owning field's block each iteration: the invariant owns(*b) includes the block.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

void buf_churn(struct buf *b, int n)
    CERASTES_REQUIRES(cerastes_owns(*b) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*b))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(cerastes_owns(*b))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(b->data);
    b->data = (int *)malloc(sizeof(int));
    if (b->data != NULL) {
      *b->data = i;
    }
    i += 1;
  }
}
// end::source[]
