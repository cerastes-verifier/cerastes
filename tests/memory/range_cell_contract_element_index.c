// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 5 (audit round 2, N1): the index of an element read in a cell contract is a parameter (p[n] with n == 0): after the call the result equals the cell's value
// Pre-registered from record 0060 item 5 (audit round 2), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int g(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && n == 0 && p[n] > 0 && p[n] < 1000)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p[n]);

int use(void)
{
  int x = 5;
  int y = g(&x, 0);
  CERASTES_CHECK(y == x);
  return y;
}
