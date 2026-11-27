// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 2, 3 and 7 (Part E; the member of the retired pin struct_array_reject_members.c L41): a structure with a matrix member u.m[i][j], written and read in a local at symbolic indexes and through a pointer under owns(*p) (the atom holds both rows' cells), and called with the address of a local; the read is the callee's result, not a contract read of the member (that form is 0064's silence, the wf pair of the member is apart)
// Pre-registered from record 0064 items 2, 3 and 7 (Part E), before any run: verifies
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

int local(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, r == 7)
{
  struct U u;
  u.n = 0;
  u.m[i][j] = 7;
  return u.m[i][j];
}

int through(struct U *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 7)
{
  p->m[i][j] = 7;
  return p->m[i][j];
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct U u = {{{0, 0, 0}, {0, 0, 0}}, 0};
  return through(&u, 1, 2);
}
