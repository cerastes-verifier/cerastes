// expect: error overflow @ L14
// mode: c23, c++20
// twin: tests/scalar/pure_clamp_plus_one.c
// Pins: 0062 item 8: without clamp100's ensures the contract's clamp100(x) + 1 may overflow (the only fact is the result's type range): one overflow error, in the contract alone (the function's own body needs no bound)
// Pre-registered from record 0062 (Part C), before the code: overflow
#include <cerastes.h>

CERASTES_PURE int clamp100(int x)
{
  return x > 100 ? 100 : x;
}

int bump(int x)
    CERASTES_ENSURES(r, r == clamp100(x) + 1)
{
  return (x > 100 ? 100 : x) + 1;
}
