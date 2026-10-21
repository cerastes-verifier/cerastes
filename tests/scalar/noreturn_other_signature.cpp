// expect: verified
// expect: native-ok
// Pins: 0015 item 5 (C++ only: C compilers know abort and exit as builtins, so a C file could not be native-ok): the table needs the exact signature: exit(void) is not exit(int)
// A user exit with another signature is an ordinary function.
#include <cerastes.h>

void exit(void)
    CERASTES_ENSURES(true);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    exit();
  }
  return x;
}
