// expect: rejected cerastes-call_in_annotation @ L15
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 1 (M4.5f1): a call of a predicate is not an application: call_in_annotation stays for a callee that is not a pure function; measured on the frozen base build, the program is rejected the same way today
// Pre-registered from record 0062 (Part B), before the code: rejected. the one test of the predicate/member pair that the frozen base build gives today (call_in_annotation at the call, both modes)
#include <cerastes.h>

CERASTES_PREDICATE bool small(int x)
{
  return x < 10;
}

int use(int y)
{
  CERASTES_CHECK(small(y));
  return y;
}
