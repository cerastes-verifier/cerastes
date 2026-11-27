// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part E; the sequencing rows of a NESTED matrix member have nothing else behind them, the nested object key carries the member steps of both structures): a call that owns only the cell p->k, and one that reads p->u.m, beside a read of p->u.m[i][j] of the same structure are accepted (disjoint cells; two reads). The twins in negative/matrix_member_nested_seq_unsequenced.c give the callee the nested structure p->u or the whole structure
// Pre-registered from record 0064 item 11 (Part E), before any run: verifies
#include <cerastes.h>

struct U {
  int n;
  int m[2][3];
};

struct V {
  struct U u;
  int k;
};

int bump_k(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(p->k))
    CERASTES_ENSURES(r, cerastes_owns(p->k) && r == 0)
{
  p->k = 1;
  return 0;
}

int peek(struct V *p)
    CERASTES_REQUIRES(cerastes_reads(p->u.m))
    CERASTES_ENSURES(r, cerastes_reads(p->u.m) && r == 0)
{
  (void)p;
  return 0;
}

int beside_cell(struct V *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return bump_k(p) < p->u.m[i][j];
}

int beside_reads(struct V *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return peek(p) < p->u.m[i][j];
}
