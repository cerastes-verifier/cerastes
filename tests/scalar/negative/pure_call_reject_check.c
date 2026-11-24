// expect: rejected cerastes-call_in_annotation @ L16
// mode: c23, c++20
// twin: tests/scalar/check_stmt.c
// Pins: 0062 item 4 (M4.5e1d Step 0 staging pin): a call of a pure function in a check is call_in_annotation until the part that lifts it; retired by Flips A with a positive of the same program
// Pre-registered from record 0062 item 4, before the code: rejected today, at the call
#include <cerastes.h>

CERASTES_PURE bool small(int x)
{
  return -100 < x && x < 100;
}

int f(int y)
    CERASTES_REQUIRES(-50 < y && y < 50)
{
  CERASTES_CHECK(small(y));
  return y;
}
