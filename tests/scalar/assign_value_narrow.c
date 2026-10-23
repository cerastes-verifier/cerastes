// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: the value of an assignment is the converted value, and the conversion is checked where the assignment is
#include <cerastes.h>

int widen(int a)
    CERASTES_REQUIRES(-32768 <= a && a <= 32767)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  short s;
  int t = (s = a) + 1;
  return t + s;
}

int chain_narrow(int a)
    CERASTES_REQUIRES(-128 <= a && a <= 127)
    CERASTES_ENSURES(r, r == 2 * a)
{
  signed char c;
  int w;
  w = c = a;
  return w + c;
}
