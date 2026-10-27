// expect: error division-by-zero @ L14
// mode: c23, c++20
// twin: tests/scalar/call_comma_divisor.c
// Pins: 0026 item 1: validate only promises y >= 0, so the divisor may be 0 after the comma
#include <cerastes.h>

// Returns only when y >= 0 (a trusted contract: the callee ends the program otherwise).
int validate(int y)
    CERASTES_ENSURES(r, y >= 0);

int ratio(int y)
    CERASTES_ENSURES(r, y <= 0 || r == 100 / y)
{
  return 100 / (validate(y), y);
}
