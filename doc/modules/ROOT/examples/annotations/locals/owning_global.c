// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// Rejected: an owning global (planned for the next version).
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int *g CERASTES_OWNING;

int f(void)
{
  return g == NULL;
}
// end::source[]
