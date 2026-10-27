// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 1 and 6: the precondition of h has the set of its argument (PRE), which holds validate(y), the call in the comma's left operand; validate ends the program unless y > 0, so h's requires y > 0 holds when C checks it
#include <cerastes.h>

// Returns only when y > 0 (a trusted contract: the callee ends the program otherwise).
int validate(int y)
    CERASTES_ENSURES(r, y > 0);

int h(int a)
    CERASTES_REQUIRES(a > 0)
    CERASTES_ENSURES(r, r == 100 / a)
{
  return 100 / a;
}

int checked(int y)
    CERASTES_ENSURES(r, 0 <= r && r <= 100)
{
  return h((validate(y), y));
}
