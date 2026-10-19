// expect: error division-by-zero @ L9
// mode: c23, c++20
// twin: tests/scalar/contract_wf.c
// Pins: 0010 item 11: a division by zero inside a requires, at the annotation's line
// The division comes before the guard, so b may be zero in the clause.
#include <cerastes.h>

int ratio(int a, int b)
    CERASTES_REQUIRES(a >= 0 && a / b >= 1 && b > 0)
    CERASTES_ENSURES(r, r >= 1)
{
  return a / b;
}

int sum_or_zero(int a, int b)
    CERASTES_REQUIRES(a >= 0 && a <= 1000000000 && b >= 0 && b <= 1000000000)
    CERASTES_ENSURES(r, r == a + b || r == 0)
{
  return a - a + b - b;
}
