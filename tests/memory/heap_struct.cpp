// expect: verified
// expect: native-ok
// Pins: 0047 item 2 (C++): the typed view is the explicit static_cast of malloc's result (C++ has no implicit conversion from void*); implicit object creation makes it sound
// Pre-registered from record 0047 item 2 (C++): verifies
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

int use_struct()
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  S *s = static_cast<S *>(malloc(sizeof(S)));
  if (s == nullptr) {
    return -1;
  }
  s->x = 1;
  s->y = 2;
  int sum = s->x + s->y;
  free(s);
  return sum;
}
