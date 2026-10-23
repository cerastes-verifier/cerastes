// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0022 item 2: a default argument is checked at each call, here an unsigned constant that fits
#include <cerastes.h>

unsigned f(unsigned a = 4294967295u)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

unsigned g(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return f();
}
