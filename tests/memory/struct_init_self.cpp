// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0043 item 6, audit S7 (C++): the elements are initialized in order, so s.x is initialized when the second element reads it (EDG's #549 is suppressed in verify mode)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int self_member()
    CERASTES_ENSURES(r, r == 1)
{
  S s = {1, s.x};
  return s.y;
}
