// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 2: the operand of sizeof is not evaluated, so it is not a read
#include <cerastes.h>

int size_of(void)
    CERASTES_ENSURES(r, r == 4)
{
  int x;
  return (int)sizeof(x);
}
