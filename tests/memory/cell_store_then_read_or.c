// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2 (audit round 2, R2-B1): with q == p the store (after tick()) in the left operand of || is complete before the right operand reads *q, so the result is 1
#include <cerastes.h>
#include <stddef.h>

int tick(void)
    CERASTES_ENSURES(r, r == 0);

int f(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p == 0)
    CERASTES_REQUIRES(q == p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 1)
{
  return ((*(tick(), p) = 5) != 5) || *q == 5;
}
