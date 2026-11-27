// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part E): the object keys of a matrix member carry both index steps and the member's path: u.m[0][0] = u.m[1][1]++ (two constant cells of a local structure) and p->m[i][j] = p->m[k][l]++ under owns(*p) and (i != k || j != l); the twin removes the distinctness
// Pre-registered from record 0064 item 11 (Part E), before any run: verifies
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

int through(struct U *p, int i, int j, int k, int l)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3 && 0 <= k && k < 2 && 0 <= l && l < 3 && (i != k || j != l))
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  if (p->m[k][l] < 0 || p->m[k][l] > 99) {
    return 0;
  }
  p->m[i][j] = p->m[k][l]++;
  return 1;
}

int consts(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U u = {{{1, 2, 3}, {4, 5, 6}}, 0};
  u.m[0][0] = u.m[1][1]++;
  CERASTES_CHECK(u.m[0][0] == 5 && u.m[1][1] == 6);
  return u.m[0][0];
}
