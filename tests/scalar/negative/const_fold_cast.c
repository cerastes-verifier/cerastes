// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/const_fold.c
// Pins: 0010 items 5 and 12: (short)70000 is checked
// EDG gives no warning for this one: the verifier is the only one that reports it.
#include <cerastes.h>

int top(void)
{
  short s = (short)70000;
  return s;
}
