// expect: error check @ L16
// twin: tests/scalar/noreturn_other_signature.cpp
// Pins: 0015 item 5 (C++ only: C compilers know abort and exit as builtins, so a C file could not be native-ok): code after a call of exit(void) is reachable, so its check is made
// exit(int) from the library would end the path.
#include <cerastes.h>

void exit(void)
    CERASTES_ENSURES(true);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    exit();
    CERASTES_CHECK(x <= 5);
  }
  return x;
}
