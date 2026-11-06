// expect: verified
// expect: native-ok
// mode: c23, c++20
// A block handed from one owner to another: t gives it to keeper, which frees it after t's scope has ended.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int handoff(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *keeper = NULL;
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
