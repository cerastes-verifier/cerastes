// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer parameter: the callee owns *p during the call and gives it back.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

void inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < INT_MAX)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}

int count_twice(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 0;
  inc(&x);
  inc(&x);
  return x;
}
// end::source[]
