// expect: error division-by-zero @ L18
// mode: c23, c++20
// twin: tests/scalar/call_comma_guard.c
// Pins: 0026 items 1 and 4 (pin): the guard (g(y), 1) carries {g}, not nr(0), the sibling of the guarded expression: d may be 0, so the division fails
#include <cerastes.h>

// Never returns.
int nr(int x)
    CERASTES_ENSURES(r, false);

// Returns y and promises nothing else.
int g(int y)
    CERASTES_ENSURES(r, r == y);

int test(int y, int d)
    CERASTES_ENSURES(r, false)
{
  return nr(0) + ((g(y), 1) && 100 / d > 0);
}
