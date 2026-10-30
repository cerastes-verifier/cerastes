// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2 (audit S6, no change): *p = bump2(p, *p): the read of *p is an argument, ordered before the call, and the store follows both; b1b's shape stays verified
#include <cerastes.h>
#include <stddef.h>

int bump2(int *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p) && v == *p && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) && r == v + 1)
{
  (void)p;
  return v + 1;
}

void use_bump2(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p = bump2(p, *p);
}
