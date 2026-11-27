// expect: error unsequenced @ L39
// expect: error unsequenced @ L46
// mode: c23, c++20
// twin: tests/memory/matrix_member_nested_seq.c
// Pins: 0064 item 11 (0039 item 3): a callee that owns the nested structure p->u (the cells p->u.m[i][j] among them) or the whole of *p, beside a read of p->u.m[i][j] in the same expression: the callee's write and the read may reach one cell. With the sequencing rows made assert true nothing else rejects either
// Pre-registered from record 0064 item 11 (Part E), before any run: one unsequenced per function, at the line of its expression
#include <cerastes.h>

struct U {
  int n;
  int m[2][3];
};

struct V {
  struct U u;
  int k;
};

int bump_u(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(p->u))
    CERASTES_ENSURES(r, cerastes_owns(p->u) && r == 0)
{
  p->u.m[0][0] = 1;
  return 0;
}

int bump_all(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  p->k = 1;
  return 0;
}

int nested_beside(struct V *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return bump_u(p) < p->u.m[i][j];  // <- here
}

int whole_beside(struct V *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return bump_all(p) < p->u.m[i][j];  // <- here
}
