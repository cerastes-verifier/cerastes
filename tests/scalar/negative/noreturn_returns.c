// expect: error noreturn @ L14
// mode: c23, c++20
// twin: tests/scalar/noreturn_abort.c
// Pins: 0015 item 4: the closing brace of a [[noreturn]] function is reachable when code == 0
// The caller passes 1, but the function is verified for every argument.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(int code)
{
  if (code != 0) {
    abort();
  }
}

int checked_div(int a, int b)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000)
    CERASTES_ENSURES(r, b != 0 && r == a / b)
{
  if (b == 0) {
    fail(1);
  }
  return a / b;
}
