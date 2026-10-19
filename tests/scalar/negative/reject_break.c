// expect: rejected cerastes-unsupported_statement @ L15
// mode: c23, c++20
// twin: tests/scalar/while_invariant.c
// Pins: 0012 item 12: break is rejected (it is a compiler-generated goto)
// The supported twin is the same loop without the break.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i > 5) {
      break;
    }
    i += 1;
  }
  return i;
}
