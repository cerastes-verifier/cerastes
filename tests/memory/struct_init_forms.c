// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 6: every complete braced form and {} store every leaf (the listed value, or zero for {}); each store sets its flag
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

int positional(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {1, 2};
  return s.x + s.y;
}

int designated_complete(void)
    CERASTES_ENSURES(r, r == 12)
{
  struct S s = {.x = 1, .y = 2};
  return s.x * 10 + s.y;
}

int empty_braces(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {};
  return s.x + s.y;
}

int nested_braces(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct T u = {{1, 2}, 3};
  return u.t.x + u.t.y + u.z;
}

int from_values(int q)
    CERASTES_REQUIRES(0 <= q && q <= 100)
    CERASTES_ENSURES(r, r == 2 * q + 1)
{
  struct S s = {q, q + 1};
  return s.x + s.y;
}
