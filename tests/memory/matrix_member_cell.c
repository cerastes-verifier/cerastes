// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 13 ("Question 348 and a matrix member": atoms on a SCALAR cell of a structure with a matrix member are accepted, as the base accepts them for a member array of structures): owns(p->n) in a contract with its caller (the auditor's m03b), written in a loop invariant (q49), on a result with a partner (qq08) and partnerless on a result that is called (qq03). The twins: negative/matrix_member_cell_wrong.c (m03c, a wrong value in the caller) and negative/matrix_member_cell_touch.c (the matrix member written under the cell's atom, in a body and in a loop)
// Pre-registered from record 0064 item 13 and the auditor's probes audit-E2a/p/m03b_owns_n.c, q49_348.c, qq08_348.c and qq03_348.c (each verified in both modes on the audited snapshot), before any run of this file: verifies
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
    CERASTES_ENSURES(r, r == 5)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  set_n(&u);
  return u.n + u.m[0][0] - 1;
}

int count(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(p->n))
    CERASTES_ENSURES(r, cerastes_owns(p->n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(p->n))
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    p->n = i;
    i = i + 1;
  }
  return 0;
}

struct U *id(struct U *p)
    CERASTES_REQUIRES(cerastes_owns(p->n))
    CERASTES_ENSURES(r, cerastes_owns(r->n) && r == p)
{
  return p;
}

struct U *mk(void)
    CERASTES_ENSURES(r, cerastes_owns(r->n));

int fresh(void)
{
  struct U *p = mk();
  p->n = 1;
  return p->n;
}
