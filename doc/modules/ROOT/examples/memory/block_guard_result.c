// expect: verified
// expect: native-ok
// mode: c23, c++20
// The result is a block exactly where *q is positive, read on return; the caller frees it when it holds it.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int *make(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(r) : r == NULL))
{
  if (*q > 0) {
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      abort();
    }
    *p = *q;
    return p;
  }
  return NULL;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 0)
{
  int c = 3;
  int *p = make(&c);
  if (p != NULL) {
    free(p);
  }
  return 0;
}
// end::source[]
