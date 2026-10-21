// expect: rejected cerastes-call_without_contract @ L11
// c-only: __builtin_expect is a GNU builtin
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: only the exact expansion of assert is recognised; another use of __builtin_expect is rejected
// (tag observed with the pre-0015 binary: __builtin_expect has no contract)
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  if (__builtin_expect(x, 0)) {
    return 1;
  }
  return 0;
}
