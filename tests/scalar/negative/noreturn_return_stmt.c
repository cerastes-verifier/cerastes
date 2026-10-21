// expect: error noreturn @ L12
// mode: c23, c++20
// twin: tests/scalar/noreturn_abort.c
// Pins: 0015 item 4: a reachable return of a [[noreturn]] function is reported at the return
// The closing brace is not reachable (both branches end the program or return), only the return is.
#include <stdlib.h>
#include <cerastes.h>

[[noreturn]] void fail(int code)
{
  if (code == 0) {
    return;
  }
  abort();
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
