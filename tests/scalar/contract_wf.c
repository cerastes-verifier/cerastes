// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 11: a contract's own arithmetic is checked, each clause under the clauses before it
// The division is guarded by the conjuncts to its left; the sum is bounded by the requires.
#include <cerastes.h>

int ratio(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b > 0 && a / b >= 1)
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
