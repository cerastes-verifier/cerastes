// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 5: f().x, g(f()), t = f(), *p = f() (every destination member written at write permission) and a discarded f()
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S mk(int a)
    CERASTES_REQUIRES(0 <= a && a < 100)
    CERASTES_ENSURES(r, r.x == a && r.y == a + 1)
{
  struct S s = {a, a + 1};
  return s;
}

int first(struct S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

void store(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == 7 && p->y == 8)
{
  *p = mk(7);
}

int use(void)
    CERASTES_ENSURES(r, r == 19)
{
  int b = mk(3).x;
  CERASTES_CHECK(b == 3);
  int a = first(mk(4));
  struct S t = {0, 0};
  t = mk(5);
  CERASTES_CHECK(t.x == 5 && t.y == 6);
  struct S u = {0, 0};
  store(&u);
  mk(9);
  return a + t.x + u.x + b;
}
