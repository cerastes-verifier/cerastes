// expect: rejected cerastes-owner_release_unowned @ L13
// mode: c23, c++20
// verify-options: --strict-owner
// Rejected by --strict-owner, although the default mode accepts it: free releases an alias of an owner.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int use(void)
{
  int *v CERASTES_OWNING = (int *)malloc(sizeof(int));
  int *t = v;
  free(t);
  return 0;
}
// end::source[]
