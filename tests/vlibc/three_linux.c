// expect: verified
// expect: native-ok
// mode: c23, c++20; platform: linux
// Pins: 0046 item 4 and 0031 item 10: the three trusted functions through the shim on glibc, whose own declarations carry `__attribute__((__noreturn__))` (ignored: "does not return" is vlibc's)
// Each of the three ends its path, so the check after them holds for every input.
#include <stdlib.h>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 9)
{
  if (x > 8) {
    abort();
  }
  if (x > 6) {
    exit(1);
  }
  if (x > 5) {
    _Exit(2);
  }
  CERASTES_CHECK(x <= 5);
  return x;
}
