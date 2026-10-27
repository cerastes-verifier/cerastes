// expect: error precondition @ L25
// mode: c23, c++20
// twin: tests/scalar/call_comma_branch.c
// Pins: 0026 items 2 and 4 (pin): inside the branch of && (its own full expression) PRE(h) is {g}, not nr: when a != 0 and y == 0 the requires of h fails
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

int test(int a, int y)
    CERASTES_ENSURES(r, 0 <= r && r <= 1)
{
  return a && (nr(0) + h((g(y), y)) > 0);
}
