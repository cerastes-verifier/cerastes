// expect: error invalid-free @ L13
// mode: c23, c++20
// verify-options: --strict-owner
// Accepted by --strict-owner (each free names an owner) and reported by the default rows of the mode: the second free.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int use(void)
{
  int *v CERASTES_OWNING = (int *)malloc(sizeof(int));
  free(v);
  free(v);
  return 0;
}
// end::source[]
