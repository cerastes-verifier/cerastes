// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/pure_ensures_twice.c
// Pins: 0062 item 7: the body adds one: the ensures r == twice(x) is the function's own value, so the postcondition error, at the return
// Pre-registered from record 0062 (Part C), before the code: postcondition
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
  return x + x + 1;
}
