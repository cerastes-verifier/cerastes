// expect: verified
// expect: native-ok
// mode: c23, c++20
// Two written atoms in one clause are separate objects: the caller of pick concludes that the result is not *p's object.
// tag::source[]
#include <cerastes.h>

static int g = 0;

int *pick(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) && cerastes_owns(*r) && r == &g)
{
  (void)p;
  return &g;
}

int main(void)
{
  int x = 1;
  int *q = pick(&x);
  *q = 2;
  CERASTES_CHECK(x == 1);
  return x - 1;
}
// end::source[]
