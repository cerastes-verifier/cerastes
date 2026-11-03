// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 4: a nested result: the leaves in struct_leaves order (res_m0_m0, res_m0_m1, res_m1), r.t.y in the contract
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  struct S t;
  int z;
};

struct T mkt(int a)
    CERASTES_REQUIRES(0 <= a && a < 100)
    CERASTES_ENSURES(r, r.t.x == a && r.t.y == a + 1 && r.z == 0)
{
  struct T w = {{a, a + 1}, 0};
  return w;
}

int use_t(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct T w = mkt(4);
  return w.t.y;
}
