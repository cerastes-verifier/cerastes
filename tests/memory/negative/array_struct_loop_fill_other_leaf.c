// expect: error check @ L26
// mode: c23, c++20
// twin: tests/memory/array_struct_loop.c
// Pins: 0064 item 10: the loop also writes the long member v[i].y (the positive's fill writes only the int member x), so the field y is reached and its values are not kept: the check after the loop that v[1].y is 4 fails (it is also false of the path n == 3)
// Pre-registered from record 0064 item 10 (Part B), before any run (revised before any run to the item's reach rule): the one error
#include <cerastes.h>

struct Q {
  int x;
  long y;
};

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == n)
{
  struct Q v[3] = {{0, 2}, {0, 4}, {0, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    v[i].x = i;
    v[i].y = 5;
    i = i + 1;
  }
  CERASTES_CHECK(v[1].y == 4 && v[2].y == 6);  // <- here
  return i + (int)(v[1].y * 0);
}
