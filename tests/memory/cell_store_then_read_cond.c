// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2 (audit round 2, R2-B1): the condition of ?: with its store is complete before the arm reads *q, so the result is 5
#include <cerastes.h>
#include <stddef.h>

int *pick(int *a)
    CERASTES_REQUIRES(cerastes_live(a))
    CERASTES_ENSURES(r, cerastes_live(a) && r == a);

int h(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && *q == 0)
    CERASTES_ENSURES(r, cerastes_owns(*q) && r == 5)
{
  return ((*pick(q) = 5) == 5) ? *q : 7;
}
