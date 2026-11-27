// expect: error unsequenced @ L20
// mode: c23, c++20
// twin: tests/memory/matrix_member_seq.c
// Pins: 0064 item 11 (0039 item 3): without (i != k || j != l) the write to p->m[i][j] and the increment of p->m[k][l] may reach one cell of the member (the path-precise row)
// Pre-registered from record 0064 item 11 (Part E), before any run: the one error
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

int through(struct U *p, int i, int j, int k, int l)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3 && 0 <= k && k < 2 && 0 <= l && l < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  if (p->m[k][l] < 0 || p->m[k][l] > 99) {
    return 0;
  }
  p->m[i][j] = p->m[k][l]++;  // <- here
  return 1;
}
