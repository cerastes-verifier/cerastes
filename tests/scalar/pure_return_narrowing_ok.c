// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 3 and 9: the conversion at the return of a pure function (here to short) is an obligation of the function; a requires that keeps the value in range gives none
// Pre-registered from record 0062 (Part B), before the code: verifies. Edited before any run (audit round 2, R2-S3): the check observes the value, so a wrong slot or swapped arguments fails; its off-by-one twin is the negative of the same name with _value_off_by_one
#include <cerastes.h>

CERASTES_PURE short narrow(int x)
    CERASTES_REQUIRES(-100 < x && x < 100)
{
  return (short)x;
}

int use(int y)
    CERASTES_REQUIRES(-50 < y && y < 50)
{
  CERASTES_CHECK(narrow(y) == y);
  return y;
}
