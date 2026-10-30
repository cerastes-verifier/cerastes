// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 4: *r in an ensures, framed by the atom owns(*p) and r == p; the caller uses *r (C spellings)
#include <cerastes.h>
#include <stddef.h>

int *pick(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p && *r == cerastes_old(*p))
{
  return p;
}

int use_pick(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  int *q = pick(&x);
  return *q;
}
