// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 5: abort, exit and _Exit never return; the switch has no path that reaches the closing brace
// Only case 0 returns, so the function needs no other return.
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
    exit(2);
  default:
    _Exit(3);
  }
}
