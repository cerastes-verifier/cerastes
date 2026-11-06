// expect: error leak @ L17
// mode: c23, c++20
// One path forgets the free: the owner still holds its block at the return.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int use_and_forget(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  return v;
}
// end::source[]
