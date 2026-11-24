// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 9: the obligations of a pure body are checked in the function; a requires that excludes the overflow gives none
// Pre-registered from record 0062 (Part B), before the code: verifies. Edited before any run (audit round 2, R2-S3): the check observes the value, so a wrong slot or swapped arguments fails; its off-by-one twin is the negative of the same name with _value_off_by_one
#include <cerastes.h>

CERASTES_PURE int sq(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
{
  return x * x;
}

int use(int y)
    CERASTES_REQUIRES(-100 < y && y < 100)
{
  CERASTES_CHECK(sq(y) == y * y);
  return y;
}
