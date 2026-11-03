// expect: verified
// expect: native-ok
// mode: c23, c++20
// Structures passed and returned by value: the members of a parameter as its entry values, the members of a result in an ensures, a member of a call's result.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

struct point make(int x, int y)
    CERASTES_ENSURES(r, r.x == x && r.y == y)
{
  struct point p = {x, y};
  return p;
}

struct point mirror(struct point p)
    CERASTES_ENSURES(r, r.x == p.y && r.y == p.x)
{
  int t = p.x;
  p.x = p.y;
  p.y = t;
  return p;
}

int sum(struct point p)
    CERASTES_REQUIRES(p.x >= 0 && p.x <= 100 && p.y >= 0 && p.y <= 100)
    CERASTES_ENSURES(r, r == p.x + p.y)
{
  return p.x + p.y;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct point a = make(1, 2);
  struct point b = mirror(a);
  return sum(b) + make(4, 0).y;
}
// end::source[]
