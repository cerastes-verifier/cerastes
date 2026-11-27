// expect: error postcondition @ L24
// expect: error out-of-bounds @ L31
// expect: error out-of-bounds @ L38
// expect: error uninitialized @ L46
// expect: error lifetime @ L53
// expect: error permission @ L63
// mode: c23, c++20
// twin: tests/memory/matrix_struct_local.c
// Pins: 0064 items 3, 6 and 17 (Part E slice 3b-1; audit round E2a, finding 3): the must-fail twin of matrix_struct_local.c, one error per function: the wrong value (r == 5 for u.m[1][0], which is 4), the first index one too high (`u.m[i][0]` with i <= 2), the second index one too high (`u.m[1][j]` with j <= 3), a cell read before it is given a value (the structure has no initializer: `u.m[0][1]`), a callee that keeps the row it was given (the structure's scope exit: `lifetime`, at the return, as the array locals have it) and a pointer to a row that outlives the block of the structure (the read is `permission`). The auditor's m01b, m06, m06b, m16, m17 and m28
// Pre-registered from record 0064 items 3, 5, 6, 8 and 10 (Part E; audit round E2a, finding 3), before any run: postcondition at L24; out-of-bounds at L31; out-of-bounds at L38; uninitialized at L46; lifetime at L53; permission at L63
#include <cerastes.h>

struct U { int n; int m[2][3]; };

void keep_row([[maybe_unused]] int *q)
    CERASTES_REQUIRES(cerastes_owns_range(q, 3))
{
}

int wrong_value(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  return u.m[1][0];  // <- here
}

int first_index_high(int i)
    CERASTES_REQUIRES(0 <= i && i <= 2)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  return u.m[i][0];  // <- here
}

int second_index_high(int j)
    CERASTES_REQUIRES(0 <= j && j <= 3)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  return u.m[1][j];  // <- here
}

int unwritten_cell(void)
{
  struct U u;
  u.n = 1;
  u.m[0][0] = 2;
  return u.m[0][1];  // <- here
}

int kept_row(void)
{
  struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
  keep_row(u.m[1]);
  return u.n;  // <- here
}

int outlives_the_block(void)
{
  int *p;
  {
    struct U u = {9, {{1, 2, 3}, {4, 5, 6}}};
    p = u.m[1];
  }
  return *p;  // <- here
}
