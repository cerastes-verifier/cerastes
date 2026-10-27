// expect: error division-by-zero @ L18
// mode: c23, c++20
// twin: tests/scalar/call_comma_divisor.c
// Pins: 0026 item 4 (pin): the comma's value carries {g}, not nr: y may be 0, so the division fails; with nr in the divisor's set the division would be checked after a call that never returns and the program would verify
#include <cerastes.h>

// Never returns.
int nr(int x)
    CERASTES_ENSURES(r, false);

// Returns y and promises nothing else.
int g(int y)
    CERASTES_ENSURES(r, r == y);

int test(int y)
    CERASTES_ENSURES(r, false)
{
  return nr(0) + 100 / (g(y), y);
}
