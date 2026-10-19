// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 5: the checks of a conditionally evaluated operand are conditional (&&, ||, ?:)
// Each division is guarded by the test on its left; none of them can divide by zero.
#include <cerastes.h>

int and_guard(int a, int b)
    CERASTES_REQUIRES(a >= 0)
    CERASTES_ENSURES(r, r == (b != 0 && a / b > 1))
{
  return b != 0 && a / b > 1;
}

int or_guard(int a, int b)
    CERASTES_REQUIRES(a >= 0)
    CERASTES_ENSURES(r, r == (b == 0 || a / b > 1))
{
  return b == 0 || a / b > 1;
}

int cond_guard(int a, int b)
    CERASTES_REQUIRES(a >= 0)
    CERASTES_ENSURES(r, r == (b != 0 ? a / b : 0))
{
  return b != 0 ? a / b : 0;
}
