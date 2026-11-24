// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 items 1 and 3: a call of a pure function with no body in the unit is rejected (M6: the contract manifest), at the call
// Pre-registered from record 0062 (Part B), before the code: rejected. Rewritten before any run (question 280): the call moved from a requires (a Part C position) to a check
#include <cerastes.h>

CERASTES_PURE bool small(int x);

int use(int y)
{
  CERASTES_CHECK(small(y));
  return y;
}
