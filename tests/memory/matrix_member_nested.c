// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 5 and 17 (Part E slice 3b-2; audit round E2a, finding 1, the auditor's q60, q61, q62 and q64, which the encoder and the lowering accept since il120): a matrix member reached through a NESTED structure, `struct V { struct U u; int k; }` with `struct U { int n; int m[2][3]; }`: under `owns(*p)` a function writes `p->u.m[1][2]` and `p->k` and states both in its `ensures`; under `owns(p->u)` it writes the cell and returns it; under `reads(p->u.m)` it reads `p->u.m[0][0]` and states it; a caller with a local `struct V` calls all three and states the cells they leave. Twin: negative/matrix_member_nested_wrong.c (a wrong value, the second index one too high, a write under `reads`, a wrong claim in the caller)
// Pre-registered from record 0064 items 3, 5 and 17 (Part E slice 3b-2; audit round E2a, finding 1: a nested structure's own facts), before any run: verifies
#include <cerastes.h>

struct U { int n; int m[2][3]; };
struct V { struct U u; int k; };

int write_all(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->u.m[1][2] == 5 && p->k == 7 && r == 7)
{
  p->u.m[1][2] = 5;
  p->k = 7;
  return p->k;
}

int write_u(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(p->u))
    CERASTES_ENSURES(r, cerastes_owns(p->u) && p->u.m[1][2] == 5 && r == 5)
{
  p->u.m[1][2] = 5;
  return p->u.m[1][2];
}

int read_matrix(struct V *p)
    CERASTES_REQUIRES(cerastes_reads(p->u.m))
    CERASTES_ENSURES(r, cerastes_reads(p->u.m) && r == p->u.m[0][0])
{
  return p->u.m[0][0];
}

int caller(void)
    CERASTES_ENSURES(r, r == 5 + 7 + 1 + 5)
{
  struct V v = {{9, {{1, 2, 3}, {4, 5, 6}}}, 3};
  int b = read_matrix(&v);
  int a = write_all(&v);
  int c = write_u(&v);
  return v.u.m[1][2] + a + b + c;
}
