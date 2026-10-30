// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5: owns(*p) written in an invariant for a parameter the function assigns (no automatic place); the loop writes through it and the ensures gets the permission back
#include <cerastes.h>
#include <stddef.h>

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}
