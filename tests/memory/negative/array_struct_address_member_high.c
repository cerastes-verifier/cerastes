// expect: error out-of-bounds @ L18
// mode: c23, c++20
// twin: tests/memory/array_struct_address.c
// Pins: 0064 item 5: &v[i].x needs the slot of v[i] to exist, so i == 3 (one past) is out-of-bounds at the address, although &v[3] alone is allowed
// Pre-registered from record 0064 item 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int member(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
    CERASTES_ENSURES(r, r == 8)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  int *q = &v[i].x;  // <- here
  *q = 8;
  CERASTES_CHECK(v[i].x == 8);
  return *q;
}
