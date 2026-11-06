// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function declared without a body may give a fresh object through its result: nothing the caller holds can be the one it names.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int *fresh(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

int use_fresh(void)
    CERASTES_ENSURES(r, r == 0 || r == 2)
{
  int *p = fresh();
  int w = 0;
  if (p != NULL) {
    *p = 2;
    w = *p;
  }
  return w;
}
// end::source[]
