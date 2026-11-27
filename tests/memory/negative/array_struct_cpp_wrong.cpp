// expect: error check @ L20
// mode: c++20
// twin: tests/memory/array_struct_cpp.cpp
// Pins: 0064 item 5: q[i].y = 9 writes the element v[i] the array sees, so the check that v[i].y is 8 fails
// Pre-registered from record 0064 item 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 0)
{
  P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  P* q = v;
  q[i].y = 9;
  CERASTES_CHECK(v[i].y == 8);  // <- here
  return v[i].y - 9;
}
