// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 3, 4, 6 (questions 104, 108): owns(*pp) && owns(**pp) on a T** parameter: the inner atom reads *pp through the atom before it; *pp == old(*pp) is the user's tie; the caller learns x == 5 through p
#include <cerastes.h>
#include <stddef.h>

void set(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == 5)
{
  **pp = 5;
}

int use_set(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 0;
  int *p = &x;
  set(&p);
  CERASTES_CHECK(x == 5);
  return x;
}
