// expect: error precondition @ L24
// mode: c23, c++20
// twin: tests/scalar/call_precondition.c
// Pins: 0012 item 6: the caller's requires y >= 0 lets y == 0 through, which the callee's x > 0 refuses (the last function of the positive)
#include <cerastes.h>

int positive(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int use(int y)
    CERASTES_REQUIRES(y > 5)
{
  return positive(y);
}

// The smallest argument the callee accepts (y == 1) is passed on: a requires of `positive` any stronger than x > 0 fails here.
int use_min(int y)
    CERASTES_REQUIRES(y >= 0)
{
  return positive(y);
}
