// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5: a conversion to an unsigned type wraps, with no error
#include <cerastes.h>

unsigned char to_uc(int x)
    CERASTES_REQUIRES(x == 257)
    CERASTES_ENSURES(r, r == 1)
{
  return x;
}

unsigned to_u(int x)
    CERASTES_REQUIRES(x == -1)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return x;
}
