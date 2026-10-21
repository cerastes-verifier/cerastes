// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: unsigned short operands are computed in int and can overflow it (faithful)
// 32768 * 65535 = 2147450880 fits int; 32769 * 65535 does not.
#include <cerastes.h>

int scale(unsigned short a)
    CERASTES_REQUIRES(a <= 32768)
    CERASTES_ENSURES(r, r >= 0 && r == a * 65535)
{
  return a * 65535;
}

// The exact result for the largest operand that fits: a weaker ensures or a stronger requires of `scale` fails here.
int scale_max(void)
    CERASTES_ENSURES(r, r == 2147450880)
{
  return scale(32768);
}
