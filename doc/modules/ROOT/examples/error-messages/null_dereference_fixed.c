// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: the pointer is tested before it is dereferenced.
// tag::source[]
#include <stddef.h>
#include <cerastes.h>

int first_or_zero(int a)
    CERASTES_ENSURES(r, r == (a > 0 ? a : 0))
{
  int x = a;
  int *p = NULL;
  if (a > 0) {
    p = &x;
  }
  if (p) {
    return *p;
  }
  return 0;
}
// end::source[]
