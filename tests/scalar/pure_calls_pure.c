// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 4 and 7: a pure function that applies another in its body; the callee's requires is a precondition row of the function, shown by its own requires
// Pre-registered from record 0062 (Part B), before the code: verifies. Edited before any run (audit round 2, R2-S3): the check observes the value, so a wrong slot or swapped arguments fails; its off-by-one twin is the negative of the same name with _value_off_by_one
#include <cerastes.h>

CERASTES_PURE int half(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, 0 <= r && r <= x)
{
  return x / 2;
}

CERASTES_PURE int halved(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return half(x);
}

int use(int y)
    CERASTES_REQUIRES(0 <= y)
{
  CERASTES_CHECK(halved(y) == y / 2);
  return y;
}
