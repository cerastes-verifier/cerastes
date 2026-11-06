// expect: error permission @ L10
// mode: c23, c++20
// An owner must receive the start of a block the function holds; the address of a local is not one.
// tag::source[]
#include <cerastes.h>

void g(void)
{
  int x = 0;
  CERASTES_OWNING int *q = &x;
  *q = 1;
}
// end::source[]
