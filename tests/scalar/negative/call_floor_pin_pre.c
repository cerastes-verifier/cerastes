// expect: error precondition @ L25
// mode: c23, c++20
// twin: tests/scalar/call_comma_validate.c
// Pins: 0026 item 4 (pin): PRE(h) is {g}, not nr: g returns y unconstrained, so h's requires fails; with a set that held nr(0) the requires would be checked after a call that never returns and the program would verify
#include <cerastes.h>

// Never returns.
int nr(int x)
    CERASTES_ENSURES(r, false);

// Returns y and promises nothing else.
int g(int y)
    CERASTES_ENSURES(r, r == y);

int h(int a)
    CERASTES_REQUIRES(a > 0)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

int test(int y)
    CERASTES_ENSURES(r, false)
{
  return nr(0) + h((g(y), y));
}
