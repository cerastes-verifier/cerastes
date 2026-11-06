// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning parameter's object is covered by the borrow the attribute stands for: the result can be that object, and the caller gets it back as it gave it.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int *identity(CERASTES_OWNING int *p)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r))
{
  return p;
}

int use_identity(void)
    CERASTES_ENSURES(r, r == 0 || r == 2)
{
  int x = 1;
  int *q = identity(&x);
  int w = 0;
  if (q != NULL) {
    *q = 2;
    w = *q;
  }
  return w;
}
// end::source[]
