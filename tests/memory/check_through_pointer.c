// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 4 (annotations): *p and p == &x are allowed in a check; the write through p is seen by the check
#include <cerastes.h>

int check_deref(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *p = &x;
  *p = 1;
  CERASTES_CHECK(*p == 1);
  CERASTES_CHECK(p == &x);
  return x;
}
