// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/const_fold.c
// Pins: 0010 items 5 and 12: a constant converting to signed char is checked
// EDG's warning #69 is suppressed in verify mode.
#include <cerastes.h>

int top(void)
{
  signed char c = 300;
  return c;
}
