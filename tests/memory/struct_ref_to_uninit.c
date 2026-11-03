// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9, 0042 item 6: ref_to_uninit on a struct S *: an object that was initialized stays initialized (per leaf) after the call
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void reset(struct S *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  p->x = 0;
}

int use_reset(void)
{
  struct S s = {1, 2};
  reset(&s);
  return s.y;
}
