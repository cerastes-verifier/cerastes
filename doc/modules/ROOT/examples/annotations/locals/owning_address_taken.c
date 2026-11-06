// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// Rejected: an owning local whose address is taken (planned for the next version).
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  CERASTES_OWNING int *p = NULL;
  int **pp = &p;
  return *pp == NULL;
}
// end::source[]
