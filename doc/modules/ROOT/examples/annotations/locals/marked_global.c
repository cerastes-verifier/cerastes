// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// Rejected: a marked global (planned for the next version).
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int *g CERASTES_REF_TO_UNINIT;

int f(void)
{
  return g == NULL;
}
// end::source[]
