// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2 (the code of the retired reject_comma.c): a left operand cast to void is accepted by the compilers
#include <cerastes.h>

int last(int a, int b)
    CERASTES_ENSURES(r, r == b)
{
  return ((void)a, b);
}
