// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 4: a [[noreturn]] function with a body verifies when no return and no closing brace is reachable
// fail() ends in abort(), so after `if (b == 0) fail();` the divisor is known to be non-zero.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(void)
{
  abort();
}

int checked_div(int a, int b)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000)
    CERASTES_ENSURES(r, b != 0 && r == a / b)
{
  if (b == 0) {
    fail();
  }
  return a / b;
}
