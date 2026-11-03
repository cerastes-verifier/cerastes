// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9, 0042 item 3: [[cerastes::uninit]] on a struct local without an initializer emits nothing; the leaves are written before they are read
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int use_uninit(void)
    CERASTES_ENSURES(r, r == 3)
{
  CERASTES_UNINIT struct S s;
  s.x = 1;
  s.y = 2;
  return s.x + s.y;
}
