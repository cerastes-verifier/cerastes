// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 4 and 5 (Part B): struct P *p = v; is the address of the first element: p[i].x = 7 with a symbolic index writes the element the array sees (the element access through a pointer takes the null, liveness and bound rows, then the cell's)
// Pre-registered from record 0064 items 4 and 5 (Part B), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int decay(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 7)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  struct P *p = v;
  p[i].x = 7;
  CERASTES_CHECK(v[i].x == 7);
  return v[i].x;
}
