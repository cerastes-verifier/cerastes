// expect: error check @ L18
// twin: tests/scalar/noreturn_user_abort.cpp
// Pins: 0015 item 5 (C++ only: C compilers know abort and exit as builtins, so a C file could not be native-ok): code after a call of a user abort() is reachable, so its check is made
// A library abort() would make the path unreachable and the check would hold vacuously.
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
    CERASTES_CHECK(x <= 5);
  }
  return x;
}
