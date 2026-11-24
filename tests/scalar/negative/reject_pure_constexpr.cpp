// expect: rejected cerastes-pure_function @ L15
// mode: c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: a constexpr pure function that is read is rejected with the new tag (M5), at the call
// Pre-registered from record 0062 (Part B), before the code: rejected. Rewritten before any run (question 280): the call moved from a requires (a Part C position) to a check
#include <cerastes.h>

CERASTES_PURE constexpr bool small(int x)
{
  return -100 < x && x < 100;
}

int use(int y)
{
  CERASTES_CHECK(small(y));
  return y;
}
