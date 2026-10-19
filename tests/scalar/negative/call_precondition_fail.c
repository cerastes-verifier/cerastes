// expect: error precondition @ L17
// mode: c23, c++20
// twin: tests/scalar/call_precondition.c
// Pins: 0012 item 6: the caller's requires does not imply the callee's
#include <cerastes.h>

int positive(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int use(int y)
    CERASTES_REQUIRES(y > -5)
{
  return positive(y);
}
