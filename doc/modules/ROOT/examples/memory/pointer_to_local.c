// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer to a local: a write through it is a write of the local.
// tag::source[]
#include <cerastes.h>

int set_through(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  int *p = &x;
  *p = 2;
  return x;
}
// end::source[]
