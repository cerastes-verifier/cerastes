// expect: rejected cerastes-annotation_reserved_library @ L7
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6 and question 117 (was 0015 item 5: code after a call of a user abort() was reachable): the global abort is the library's name, so its user definition is rejected before any call is looked at
// The library abort() ends the path (tests/vlibc/abort_shim.c); a user one with a contract is refused.
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
