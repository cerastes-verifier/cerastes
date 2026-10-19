// expect: error overflow @ L17
// mode: c23, c++20
// twin: tests/scalar/contract_wf.c
// Pins: 0010 item 11: signed overflow inside an ensures, at the annotation's line
// Without bounds on a and b the first disjunct a + b can overflow; the second disjunct makes
// the postcondition itself hold, so the well-formedness is the only error.
#include <cerastes.h>

int ratio(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b > 0 && a / b >= 1)
    CERASTES_ENSURES(r, r >= 1)
{
  return a / b;
}

int sum_or_zero(int a, int b)
    CERASTES_ENSURES(r, r == a + b || r == 0)
{
  return 0;
}
