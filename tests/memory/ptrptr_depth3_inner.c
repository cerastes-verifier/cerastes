// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 2, 3: places at depth 3 (***ppp): each inner atom reads the cell the atom before it owns
#include <cerastes.h>
#include <stddef.h>

void set3(int ***ppp)
    CERASTES_REQUIRES(cerastes_owns(*ppp) && cerastes_owns(**ppp) && cerastes_owns(***ppp))
    CERASTES_ENSURES(cerastes_owns(*ppp) && *ppp == cerastes_old(*ppp) && cerastes_owns(**ppp) && **ppp == cerastes_old(**ppp) && cerastes_owns(***ppp) && ***ppp == 7)
{
  ***ppp = 7;
}

int use3(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  set3(&pp);
  return x;
}
