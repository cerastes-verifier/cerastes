// expect: error division-by-zero @ L19
// mode: c23, c++20
// twin: tests/scalar/call_comma_divisor.c
// Pins: 0026 items 1 and 4 (pin): the write of a comma's value, x = (g(y), y), is ordered after g and not after the sibling nr(0): x may be 0 when 100 / x is checked
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
  int x = 1;
  return nr(0) + ((x = (g(y), y)), 100 / x);
}
