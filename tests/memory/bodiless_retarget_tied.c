// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 5 (audit B1): a contract without a body whose ensures atom live(*pp) is tied (*pp == old(*pp)): f__wf's body-less check passes and callers prove p == &x
#include <cerastes.h>
#include <stddef.h>

void retarget(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_live(*pp));

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int y = 0;
  int *p = &x;
  retarget(&p, &y);
  CERASTES_CHECK(p == &x);
  return 0;
}
