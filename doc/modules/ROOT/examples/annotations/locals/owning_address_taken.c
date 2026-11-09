// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning local whose address is taken: the block is freed through the alias, after which the owner holds nothing.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  int **pp = &p;
  free(*pp);
  return 0;
}
// end::source[]
