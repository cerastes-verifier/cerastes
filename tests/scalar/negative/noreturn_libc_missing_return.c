// expect: error missing-return @ L23
// mode: c23, c++20
// twin: tests/scalar/noreturn_libc.c
// Pins: 0015 item 5: a case that only breaks (no exit call) reaches the closing brace
// Replacing exit(2) by a break makes the path of code == 2 fall off the end.
#include <stdlib.h>
#include <cerastes.h>

int dispatch(int code)
    CERASTES_REQUIRES(0 <= code && code <= 3)
    CERASTES_ENSURES(r, code == 0 && r == 7)
{
  switch (code) {
  case 0:
    return 7;
  case 1:
    abort();
  case 2:
    break;
  default:
    _Exit(3);
  }
}
