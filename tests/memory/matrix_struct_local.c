// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 6 and 17 (Part E slice 3b-1; audit round E2a, finding 3: a rule with no C-level test): a LOCAL structure with a matrix member, `struct U { int n; int m[2][3]; }`: the allocation with the nested initializer and the zero fill (`{9, {{1, 2, 3}, {4}}}`: row 1 is {4, 0, 0}; `{0}`: every cell zero), `u.m[i][j]` written and read at symbolic indexes, a row `u.m[1]` passed to a scalar range callee that gives it back (and one that only reads it), and the member through a pointer to the structure. Every scope exit holds: the callee lent the row and gave it back. Twins: negative/matrix_struct_local_wrong.c (the wrong value, both indexes one too high, an uninitialized cell, a callee that keeps the row, a pointer that outlives the block); derived from the auditor's m01, m05, m16, m17 and m28
// Pre-registered from record 0064 items 3, 5, 6, 8 and 10 (Part E; audit round E2a, finding 3), before any run: verifies
#include <cerastes.h>

struct U { int n; int m[2][3]; };

void zero_first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  p[0] = 0;
}

int read_first(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == 0)
{
  (void)p;
  (void)n;
  return 0;
}

int zero_fill(void)
    CERASTES_ENSURES(r, r == 4 + 9)
{
  struct U u = {9, {{1, 2, 3}, {4}}};
  return u.m[1][0] + u.m[1][1] + u.m[1][2] + u.n;
}

int all_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct U u = {0, {{0}}};
  return u.n + u.m[0][0] + u.m[1][2];
}

int cell_rw(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, r == 7 + 9)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  u.m[i][j] = 7;
  return u.m[i][j] + u.n;
}

int row_to_callee(void)
    CERASTES_ENSURES(r, r == 0 + 1 + 9)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  zero_first(u.m[1], 3);
  return u.m[1][0] + u.m[0][0] + u.n;
}

int row_to_reader(void)
    CERASTES_ENSURES(r, r == 6 + 4 + 9)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  int z = read_first(u.m[1], 3);
  return u.m[1][2] + u.m[1][0] + u.n + z;
}

int through_pointer(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  struct U *p = &u;
  p->m[1][2] = 5;
  return u.m[1][2];
}
