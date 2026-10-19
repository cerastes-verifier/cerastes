// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/const_fold.c
// Pins: 0010 item 12: INT_MAX + 1 is not folded and is an overflow
// EDG's own warning #61 is suppressed in verify mode (the verifier reports it once).
#include <cerastes.h>

int top(void)
{
  return 2147483647 + 1;
}
