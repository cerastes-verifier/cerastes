// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 3, 6: old(**pp) is the entry value of the entry object; the caller reads x after inc2(&p)
#include <cerastes.h>
#include <stddef.h>

void inc2(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp < 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == cerastes_old(**pp) + 1)
{
  **pp += 1;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  int *p = &x;
  inc2(&p);
  return x;
}
