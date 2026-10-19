// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 7: a zero divisor and MIN / -1 are errors; a positive divisor is neither
#include <cerastes.h>

int divide(int a, int b)
    CERASTES_REQUIRES(b > 0)
{
  return a / b;
}
