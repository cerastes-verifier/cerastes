// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 items 5, 7: compound literals as values: t = (struct S){1, 2}, a literal argument with designators, a literal returned (needs the encoder of commit 5 for the literal itself; the file stays verified)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S mk(void)
    CERASTES_ENSURES(r, r.x == 5 && r.y == 6)
{
  return (struct S){5, 6};
}

int get(struct S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int use(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct S t = {0, 0};
  t = (struct S){1, 2};
  CERASTES_CHECK(t.x == 1 && t.y == 2);
  int a = get((struct S){.x = 3, .y = 2});
  struct S m = mk();
  return t.x + a + m.x;
}
