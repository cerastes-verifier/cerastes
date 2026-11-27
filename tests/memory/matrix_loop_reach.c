// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 5 and 10 (Part E; audit round E2a, finding 3: the loops' reach and invariant reads had no C-level test): over a LOCAL matrix, a loop that only READS the matrix keeps every value (a read by name; a callee that reads the matrix whole; a callee that reads one row), a loop invariant may read `m[e][e2]` (here `m[1][2] == 6` and `m[i][0] >= 1` under `i < 2`), nested loops with both indexes symbolic keep their facts, and a measure may read a cell. Twins: negative/matrix_loop_reach_local.c (a store by name, a store through `int *p = m[1]`, through a pointer to a cell, an owning callee over rows and over one row each LOSE what the loop does not state) and negative/matrix_loop_reads_bounds.c (the reads outside the matrix, an unwritten cell, a value the loop breaks, a wrong measure); the auditor's l02b, l04b, l05b, l06, l14 and l15
// Pre-registered from record 0064 items 5 and 10 (Part E; audit round E2a, finding 3), before any run: verifies
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

int name_read_keeps(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    s = m[i][0];
    i = i + 1;
  }
  CERASTES_CHECK(m[1][2] == 6 && m[0][0] == 1);
  return s;
}

int callee_reads_rows_keeps(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    s = rmat(m, 2);
    i = i + 1;
  }
  CERASTES_CHECK(m[1][2] == 6 && m[0][0] == 1);
  return s;
}

int callee_reads_row_keeps(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    s = rrow(m[i], 3);
    i = i + 1;
  }
  CERASTES_CHECK(m[1][2] == 6);
  return s;
}

int invariant_reads_cells(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && m[1][2] == 6 && (i == 2 || m[i][0] >= 1))
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return i + m[0][0] * 0;
}

int nested_symbolic(void)
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 3 && 0 <= i && i < 2 && (j == 0 || m[i][j - 1] == 0))
    CERASTES_DECREASES(3 - j)
    while (j < 3) {
      m[i][j] = 0;
      j = j + 1;
    }
    CERASTES_CHECK(m[i][2] == 0);
    i = i + 1;
  }
  return m[1][2] * 0;
}

int measure_reads_a_cell(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  CERASTES_INVARIANT(0 <= m[1][2] && m[1][2] <= 6)
  CERASTES_DECREASES(m[1][2])
  while (m[1][2] > 0) {
    m[1][2] = m[1][2] - 1;
  }
  return m[1][2];
}
