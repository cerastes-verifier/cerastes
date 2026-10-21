// expect: verified
// expect: native-ok
// Pins: 0015 item 5 (C++ only: C compilers know abort and exit as builtins, so a C file could not be native-ok): the libc table applies only to the library's functions; a user abort() is an ordinary function
// This abort returns (its contract is `true`), so nothing is known after the call beyond the code before it.
#include <cerastes.h>

void abort(void) CERASTES_ENSURES(true)
{
  return;
}

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 5) {
    abort();
  }
  return x;
}
