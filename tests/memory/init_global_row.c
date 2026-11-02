// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 7 (audit S3): f holds g's cell through p with an unknown flag; after *p = 1 the flag is set, so the call of h, which needs owns(g), is fine
#include <cerastes.h>

static int g = 0;

void h(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  g = 1;
}

void f(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 1;
  h();
}

int main(void)
{
  return g;
}
