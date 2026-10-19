// expect: error overflow @ L13
// mode: c23, c++20
// twin: tests/scalar/add_bounds.c
// Pins: 0010 item 3: signed addition may overflow, reported at the operator
// Only a lower bound: a + b can pass INT_MAX. (The ensures has no arithmetic, so the
// contract's own well-formedness is not a second error.)
#include <cerastes.h>

int add(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r >= a)
{
  return a + b;
}
