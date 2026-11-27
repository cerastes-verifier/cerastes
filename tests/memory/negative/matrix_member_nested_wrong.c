// expect: error postcondition @ L20
// expect: error out-of-bounds @ L27
// expect: error permission @ L34
// expect: error postcondition @ L51
// mode: c23, c++20
// twin: tests/memory/matrix_member_nested.c
// Pins: 0064 items 3, 5 and 17 (Part E slice 3b-2; audit round E2a, finding 1): the must-fail twin of matrix_member_nested.c, one error per function: a wrong value in the `ensures` of the function under `owns(*p)` (`p->u.m[1][2] == 6` after writing 5: `postcondition`), the second index one too high on the nested matrix (`p->u.m[1][j]` with `j <= 3`: `out-of-bounds`), a write to a cell of the nested matrix under `reads(p->u.m)` (`permission`, at the write) and a caller whose claim about the cell the callee wrote is wrong (`postcondition`)
// Pre-registered from record 0064 items 3, 5 and 17 (Part E slice 3b-2; audit round E2a, finding 1: a nested structure's own facts), before any run: postcondition at L20; out-of-bounds at L27; permission at L34; postcondition at L51
#include <cerastes.h>

struct U { int n; int m[2][3]; };
struct V { struct U u; int k; };

int write_all(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->u.m[1][2] == 6 && p->k == 7 && r == 7)
{
  p->u.m[1][2] = 5;
  p->k = 7;
  return p->k;  // <- here
}

int second_index_high(struct V *p, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= j && j <= 3)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p->u.m[1][j];  // <- here
}

int write_under_reads(struct V *p)
    CERASTES_REQUIRES(cerastes_reads(p->u.m))
    CERASTES_ENSURES(r, cerastes_reads(p->u.m))
{
  p->u.m[0][0] = 1;  // <- here
  return 0;
}

int write_u(struct V *p)
    CERASTES_REQUIRES(cerastes_owns(p->u))
    CERASTES_ENSURES(r, cerastes_owns(p->u) && p->u.m[1][2] == 5 && r == 5)
{
  p->u.m[1][2] = 5;
  return p->u.m[1][2];
}

int caller(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct V v = {{9, {{1, 2, 3}, {4, 5, 6}}}, 3};
  int c = write_u(&v);
  return v.u.m[1][2] + c * 0;  // <- here
}
