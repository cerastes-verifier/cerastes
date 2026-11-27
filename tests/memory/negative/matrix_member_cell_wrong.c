// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/memory/matrix_member_cell.c
// Pins: 0064 item 13: the atom on the scalar cell p->n gives a callee the cell and nothing else, and the caller knows what the callee wrote: set_n writes 5 and the claim r == 6 is the one wrong line (the auditor's m03c)
// Pre-registered from record 0064 item 13 and the auditor's probe audit-E2a/p/m03c_owns_n_wrong.c (postcondition at the return line in both modes), before any run: the one error
#include <cerastes.h>

struct U {
  int n;
  int m[2][3];
};

void set_n(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(p->n))
    CERASTES_ENSURES(cerastes_owns(p->n) && p->n == 5)
{
  p->n = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  set_n(&u);
  return u.n + u.m[0][0] - 1;
}
