// expect: verified
// expect: native-ok
// mode: c23, c++20
// A loop that replaces the owning field's block through a second, non-owning pointer v = p to a borrowed structure: the invariant names the parameter, owns(*p) (decision 0056 item 8).
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int len;
};

void buf_churn_alias(struct buf *p, int n)
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
// end::source[]
