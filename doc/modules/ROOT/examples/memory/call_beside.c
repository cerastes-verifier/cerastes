// expect: verified
// expect: native-ok
// mode: c23, c++20
// A call that owns *p beside accesses of other cells, and a store that C orders after the call.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int beside(void)
    CERASTES_ENSURES(r, r == 11)
{
  int x = 1;
  int y = 2;
  int s = x + bump(&y);
  x = bump(&x);
  int t = bump(&x) + bump(&y);
  return s + t;
}
// end::source[]
