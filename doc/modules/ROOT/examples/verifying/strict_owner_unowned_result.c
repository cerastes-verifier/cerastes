// expect: rejected cerastes-owner_result_unowned @ L20
// mode: c23, c++20
// verify-options: --strict-owner
// Rejected by --strict-owner: the result of an owning function goes to a local that is no owner.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *create(void)
{
  int *a CERASTES_OWNING = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

int use(void)
{
  int *t = create();
  return t == NULL;
}
// end::source[]
