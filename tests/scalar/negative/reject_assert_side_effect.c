// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: the condition of an assert is code; a side effect in it (x++ as a value) is rejected
#include <assert.h>
#include <cerastes.h>

int effect(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  assert(x++);
  return x;
}
