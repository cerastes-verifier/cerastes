// expect: error division-by-zero @ L14
// mode: c23, c++20
// twin: tests/scalar/call_comma_guard.c
// Pins: 0026 item 4 (pin): the floor of a comma does not leak to its sibling: C may divide before the call that never returns, so d == 0 fails
#include <cerastes.h>

// Never returns.
int nr(int x)
    CERASTES_ENSURES(r, false);

int test(int d)
    CERASTES_ENSURES(r, false)
{
  return (nr(0), 1) + 100 / d;
}
