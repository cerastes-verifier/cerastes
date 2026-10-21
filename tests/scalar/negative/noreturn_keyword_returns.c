// expect: error noreturn @ L14
// c-only: _Noreturn is the C11 keyword; C++ has no such keyword (the attribute is [[noreturn]])
// twin: tests/scalar/noreturn_keyword.c
// Pins: 0015 item 4: a _Noreturn function that may return is reported like a [[noreturn]] one
// code == 0 skips abort and reaches the closing brace.
#include <stdlib.h>
#include <cerastes.h>

_Noreturn void bail(int code)
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
    bail(1);
  }
  return a / b;
}
