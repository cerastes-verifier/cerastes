// expect: error precondition @ L31
// mode: c23, c++20
// twin: tests/scalar/call_comma_validate.c
// Pins: 0026 item 4 (pin): a nested call: PRE(h) is {g}, not nr, although h's result is an argument of h2 next to nr(0): y may be 0, so h's requires fails
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

int h2(int p, int q)
    CERASTES_ENSURES(r, r == q)
{
  return q + (p - p);
}

int test(int y)
    CERASTES_ENSURES(r, false)
{
  return h2(nr(0), h((g(y), y)));
}
