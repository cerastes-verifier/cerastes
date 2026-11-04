// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 6 (question 111): two calls with load-rooted contracts in one expression, whose objects are distinct: accepted (0039 item 4)
#include <cerastes.h>
#include <stddef.h>

int bump(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && **pp < 100)
    CERASTES_ENSURES(r, cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && **pp == cerastes_old(**pp) + 1 && r == **pp)
{
  **pp += 1;
  return **pp;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  int z = 2;
  int *p = &x;
  int *q = &z;
  int y = bump(&p) + bump(&q);
  return y;
}
