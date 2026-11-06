// expect: verified
// expect: native-ok
// mode: c23, c++20
// The function takes the block exactly where *q is positive, read on entry, and frees it under the same test.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

void maybe_free(int *q, int *p)
    CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(p) : true))
    CERASTES_ENSURES(cerastes_owns(*q))
{
  if (*q > 0) {
    free(p);
  }
}

int use_guard(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  int c = 1;
  maybe_free(&c, p);
  int d = 0;
  maybe_free(&d, NULL);
  return 0;
}
// end::source[]
