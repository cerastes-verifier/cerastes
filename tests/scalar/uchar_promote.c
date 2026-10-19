// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: unsigned char operands promote to int; a result converted back wraps
#include <cerastes.h>

int sum_uc(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

unsigned char wrap_uc(unsigned char a)
    CERASTES_REQUIRES(a == 255)
    CERASTES_ENSURES(r, r == 0)
{
  return a + 1;
}
