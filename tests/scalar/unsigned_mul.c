// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 4: unsigned multiplication wraps
#include <cerastes.h>

unsigned mul_u(unsigned a)
    CERASTES_REQUIRES(a == 65536u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a * 65536u;
}
