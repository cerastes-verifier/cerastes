// expect: verified
// expect: native-ok
// mode: c23, c++20
// Passing the block to a function that takes it ends the owner's obligation.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
  free(p);
}

int make_and_dispose(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 5;
  dispose(p);
  return 0;
}
// end::source[]
