// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/const_fold.c
// Pins: 0010 items 5 and 12: return 300 from a signed char function
// EDG's warning #69 is suppressed in verify mode.
#include <cerastes.h>

signed char top(void)
{
  return 300;
}
