// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 10: unary minus on an unsigned type wraps
#include <cerastes.h>

unsigned negate_u(unsigned a)
    CERASTES_REQUIRES(a == 1u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return -a;
}
