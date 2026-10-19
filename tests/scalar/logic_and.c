// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 10: && yields 0 or 1 (an int in C, a bool in C++)
#include <cerastes.h>

int in_range(int a)
    CERASTES_ENSURES(r, r == (a > 0 && a < 10))
{
  return a > 0 && a < 10;
}
