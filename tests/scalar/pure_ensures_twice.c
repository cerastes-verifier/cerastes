// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 4 and 7: an ordinary function's ensures applies a pure function (r == twice(x)): the application's value is the function's body, so a body that doubles proves it
// Pre-registered from record 0062 (Part C), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int twice(int x)
    CERASTES_REQUIRES(-1000 <= x && x <= 1000)
{
  return x + x;
}

int dbl(int x)
    CERASTES_REQUIRES(-1000 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == twice(x))
{
  return x + x;
}
