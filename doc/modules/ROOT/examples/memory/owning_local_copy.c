// expect: error leak @ L19
// mode: c23, c++20
// A copy in a plain pointer does not take the obligation: t still holds the block when its scope ends, though keeper frees it later.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int copy_to_plain(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *keeper = NULL;
  {
    CERASTES_OWNING int *t = (int *)malloc(sizeof(int));
    if (t == NULL) {
      return -1;
    }
    *t = 3;
    keeper = t;
  }
  free(keeper);
  return 0;
}
// end::source[]
