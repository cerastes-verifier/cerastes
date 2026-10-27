// expect: error division-by-zero @ L14
// mode: c23, c++20
// twin: tests/scalar/call_comma_guard.c
// Pins: 0026 item 1: nonzero returns for d == 0 now, so the division is reached with d == 0
#include <cerastes.h>

// Returns its argument.
int nonzero(int d)
    CERASTES_ENSURES(r, r == d);

int guard(int d)
    CERASTES_ENSURES(r, d == 0 || r == (100 / d != 0))
{
  return (nonzero(d), 1) && 100 / d;
}
