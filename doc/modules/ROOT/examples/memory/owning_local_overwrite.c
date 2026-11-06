// expect: error leak @ L16
// mode: c23, c++20
// Assigning to an owning local while it holds a block drops the block.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int overwrite(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  p = (int *)malloc(sizeof(int));
  free(p);
  return 0;
}
// end::source[]
