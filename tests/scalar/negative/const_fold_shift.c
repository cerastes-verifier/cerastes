// expect: error shift @ L10
// mode: c23, c++20
// twin: tests/scalar/const_fold.c
// Pins: 0010 item 12: 1 << 40 is folded with its expression kept; the shift is checked
// EDG's warning #63 is suppressed in verify mode.
#include <cerastes.h>

int top(void)
{
  return 1 << 40;
}
