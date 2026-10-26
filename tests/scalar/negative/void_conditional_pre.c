// expect: error precondition @ L28
// mode: c23, c++20
// twin: tests/scalar/void_conditional.c
// Pins: 0025 item 3: the precondition of neg(y) is checked on the else path only, and it fails there (y < 5 does not give y < 0); pos(x) holds on its own path
#include <cerastes.h>

void pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  (void)x;
}

void neg(int y)
    CERASTES_REQUIRES(y < 0)
{
  (void)y;
}

int id(int a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

int statement(int c, int x, int y)
    CERASTES_REQUIRES(c ? x > 0 : y < 5)
{
  c ? pos(x) : neg(y);
  return c;
}
