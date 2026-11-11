// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 10 and 0037: f(a) where f owns *p and writes p[0]: the subscript of a parameter at index 0 needs nothing beyond the atom (S43 (i): the emitted owns atom carries offset_of < len)
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

void set0(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 9)
{
  p[0] = 9;
}

int use(void)
    CERASTES_ENSURES(r, r == 9)
{
  int a[3] = {1, 2, 3};
  set0(a);
  return a[0];
}
