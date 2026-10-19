// expect: verified
// expect: native-ok
// Pins: 0012 item 6 (P7): a default argument whose own expression is arithmetic is checked at the call (C++)
#include <cerastes.h>

int pick(int a, int b = 1000 * 1000)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == a)
{
  (void)b;
  return a;
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  return pick(1);
}
