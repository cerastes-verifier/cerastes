// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: in C the operand of ! is a plain int read as e != 0
#include <cerastes.h>

int is_zero(int a)
    CERASTES_ENSURES(r, r == (a == 0))
{
  return !a;
}
