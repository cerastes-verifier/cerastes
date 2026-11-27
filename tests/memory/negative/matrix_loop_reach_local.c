// expect: error check @ L55
// expect: error check @ L70
// expect: error check @ L87
// expect: error check @ L101
// expect: error check @ L115
// mode: c23, c++20
// twin: tests/memory/matrix_loop_reach.c
// Pins: 0064 items 5 and 10 (Part E; audit round E2a, finding 3): the must-fail twin of matrix_loop_reach.c for the REACH of a loop over a local matrix: what the loop writes (or lets a callee write) it no longer knows, and a loop that does not state a value keeps none of the matrix's. One check per function fails: a store by name (`m[i][0] = 0`, then `m[1][2] == 6`), a store through `int *p = m[1]` (then `m[0][0] == 1`), a store through a pointer to a cell, an owning callee over the rows (`wmat(m, 2)`) and over one row (`wrow(m[i], 3)`); the auditor's l02, l03, l03b, l04 and l05
// Pre-registered from record 0064 items 5 and 10 (Part E; audit round E2a, finding 3), before any run: check at L55; check at L70; check at L87; check at L101; check at L115
#include <cerastes.h>

void wrow(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0] = 0;
  (void)n;
}

int rrow(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == 0)
{
  (void)p;
  (void)n;
  return 0;
}

void wmat(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  a[0][0] = 0;
  (void)n;
}

int rmat(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == 0)
{
  (void)n;
  return a[0][0] * 0;
}

int store_by_name(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    m[i][0] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(m[1][2] == 6);  // <- here
  return m[0][0] * 0;
}

int store_through_a_row_pointer(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int *p = m[1];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    p[i] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(m[0][0] == 1);  // <- here
  return 0;
}

int store_through_a_cell_pointer(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    p = &m[1][i];
    *p = 0;
    i = i + 1;
  }
  CERASTES_CHECK(m[1][0] == 4);  // <- here
  return 0;
}

int owning_callee_over_rows(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    wmat(m, 2);
    i = i + 1;
  }
  CERASTES_CHECK(m[1][2] == 6);  // <- here
  return 0;
}

int owning_callee_over_a_row(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    wrow(m[i], 3);
    i = i + 1;
  }
  CERASTES_CHECK(m[1][2] == 6);  // <- here
  return 0;
}
