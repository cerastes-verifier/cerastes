// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: the local that receives malloc's result is not marked.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *p = (int *)malloc(sizeof(int));
  free(p);
  return 0;
}
// end::source[]
