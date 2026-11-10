// expect: error check @ L21
// mode: c23, c++20
// twin: tests/memory/not_invalidating_pointer_to_pointer.c
// Pins: 0058 item 2: the same program without the marker: the contract says nothing about *pp afterwards, so q may point anywhere and the check q == &x may not hold
// Pre-registered from record 0058 item 2: one check error at the check (the unmarked contract is accepted today; the failing check is the one reason)
#include <cerastes.h>

void zero(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp))
{
  **pp = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 5;
  int *q = &x;
  zero(&q);
  CERASTES_CHECK(q == &x);  // <- here
  return 0;
}
