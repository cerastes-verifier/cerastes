// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 6: a call asserts the callee's requires, at the call
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
    CERASTES_REQUIRES(y > 0)
{
  return positive(y);
}
