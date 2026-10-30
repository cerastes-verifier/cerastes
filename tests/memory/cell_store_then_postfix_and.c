// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2 (audit round 2, R2-B1): the store of 5 completes first, then (*q)++ in the right operand of && makes *q 6
#include <cerastes.h>
#include <stddef.h>

int *pick(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == a);

void g(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 0)
    CERASTES_ENSURES(cerastes_owns(*q) && *q == 6)
{
  int s = ((*pick(q) = 5) == 5) && (*q)++;
  (void)s;
}
