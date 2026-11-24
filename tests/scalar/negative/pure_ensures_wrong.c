// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/pure_ensures_ok.c
// Pins: 0062 items 2, 9 and 11: nothing calls this pure function and its ensures is wrong: the postcondition error, at the function's return, with a note at the clause
// Pre-registered from record 0062 (Part B), before the code: postcondition
#include <cerastes.h>

CERASTES_PURE int dbl(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
    CERASTES_ENSURES(r, r == x + x + 1)
{
  return x + x;
}
