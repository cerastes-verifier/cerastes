// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: the call is its own statement, so the read of x follows it.
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

int sum(void)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 1;
  int v = bump(&x);
  return x + v;
}
// end::source[]
