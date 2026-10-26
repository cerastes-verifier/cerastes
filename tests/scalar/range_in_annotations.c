// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 6 R10: a constant outside the range of the other operand's type in an annotation is not rejected (code only)
#include <cerastes.h>

// 0025 item 6 R10 applies to code only: the compilers never see a contract
unsigned char same(unsigned char c)
    CERASTES_ENSURES(r, r != 300 && r == c)
{
  return c;
}
