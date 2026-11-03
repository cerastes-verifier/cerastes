// expect: verified
// expect: native-ok
// c-only: a partial designated list is -Wmissing-field-initializers in g++ -Wextra, so it cannot be native-ok in C++
// Pins: 0043 item 6: a designated list and C's {0} zero-fill the leaves they leave out
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

int designated(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct S s = {.y = 2};
  CERASTES_CHECK(s.x == 0);
  return s.x + s.y;
}

int zero_braces(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {0};
  return s.x + s.y;
}

int nested_designated(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct T u = {.t = {.y = 5}, .z = 4};
  CERASTES_CHECK(u.t.x == 0);
  return u.t.y + u.z;
}
