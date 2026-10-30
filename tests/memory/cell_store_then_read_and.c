// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2 (audit round 2, R2-B1): the store through pick(q) in the left operand of && is complete before the right operand reads *q, so the result is 1
#include <cerastes.h>
#include <stddef.h>

int *pick(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == a);

int f(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 0)
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 1)
{
  return ((*pick(q) = 5) == 5) && *q == 5;
}
