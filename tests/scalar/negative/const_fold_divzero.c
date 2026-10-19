// expect: error division-by-zero @ L10
// mode: c23, c++20
// twin: tests/scalar/const_fold.c
// Pins: 0010 item 12: 1 / 0 is not folded and is a division by zero
// EDG's warning #39 is suppressed in verify mode.
#include <cerastes.h>

int top(void)
{
  return 1 / 0;
}
