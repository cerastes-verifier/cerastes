// expect: error unsequenced @ L18
// mode: c23, c++20
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
{
  int x = 1;
  return x + bump(&x);
}
// end::source[]
