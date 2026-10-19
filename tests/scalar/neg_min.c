// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 3: unary minus of INT_MIN overflows
// The requires excludes INT_MIN (written -2147483647 - 1 would be an expression; the bound
// -2147483647 already leaves it out).
#include <cerastes.h>

int negate(int a)
    CERASTES_REQUIRES(a >= -2147483647)
    CERASTES_ENSURES(r, r == -a)
{
  return -a;
}
