// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 6 (question 111): a call with a load-rooted contract beside another access in one expression (x + bump(&p)) is accepted when **pp cannot be x: 0039 item 4's checks
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
    CERASTES_ENSURES(r, r == 7)
{
  int x = 1;
  int z = 5;
  int *p = &z;
  int y = x + bump(&p);
  return y;
}
