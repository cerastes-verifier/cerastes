// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 8 fact (ii): a later clause rests on the callee's ensures: clamp100(x) + 1 cannot overflow because clamp100 promises r <= 100
// Pre-registered from record 0062 (Part C), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int clamp100(int x)
    CERASTES_ENSURES(r, r <= 100)
{
  return x > 100 ? 100 : x;
}

int bump(int x)
    CERASTES_ENSURES(r, r == clamp100(x) + 1)
{
  return (x > 100 ? 100 : x) + 1;
}
