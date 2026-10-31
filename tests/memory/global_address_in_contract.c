// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 7 (audit S8): &g as a value in a contract (r == &g): a constant Ref, comparisons are identity
#include <cerastes.h>

static int g = 3;
static int h = 7;

int *addr_g(void)
    CERASTES_ENSURES(r, r == &g)
{
  return &g;
}

int *addr_h(void)
    CERASTES_ENSURES(r, r == &h)
{
  return &h;
}
