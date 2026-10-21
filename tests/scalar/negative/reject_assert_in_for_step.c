// expect: rejected cerastes-unsupported_expression @ L14
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 item 6: an assert is a statement; in the step of a for loop it is rejected
#include <assert.h>
#include <cerastes.h>

int steps(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (; i < n; assert(i < 100)) {
    i += 1;
  }
  return i;
}
