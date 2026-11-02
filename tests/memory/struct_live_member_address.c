// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 5, 8: live(p) is WF_S(p): &p->x and &p->y are in-bounds pointers into different member blocks (neither one past the end), so == is decided: false
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int same(struct S *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(p) && r == 0)
{
  return &p->x == &p->y;
}
