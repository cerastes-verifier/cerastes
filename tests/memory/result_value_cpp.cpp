// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0040 item 4: *r in an ensures, framed by the atom owns(*p) and r == p; the caller uses *r (C++ spellings)
#include <cerastes.h>
#include <stddef.h>

int *pick(int *p)
    CERASTES_REQUIRES(cerastes::owns(*p))
    CERASTES_ENSURES(r, cerastes::owns(*p) && r == p && *r == cerastes::old(*p))
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
