// expect: verified
// expect: native-ok
// mode: c23, c++20
// A structure passed and returned by value: the members of the result are named in the contract.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

struct point mirrored(struct point a)
    CERASTES_ENSURES(r, r.x == a.y && r.y == a.x)
{
  struct point m = {a.y, a.x};
  return m;
}

int use_mirrored(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct point a = {3, 4};
  struct point b = mirrored(a);
  return b.y;
}
// end::source[]
