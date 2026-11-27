// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 8 (audit round E1, finding 4; the pairing rows against a READS site and from a FORWARDER, the controls of negative/array_rows_alias_reads_overlap.c): a callee that owns the first row of a matrix beside a reads_range of one cell or of another row, called with the first row and a cell or the second row of the same matrix (disjoint), verifies; and a function that forwards part of its own range to such a callee (a[0..1) and the cell a[1][2], a[0..1) and the row a[1]) verifies, the two atoms being disjoint. At most two arrays of aggregates per function (the cost rule)
// Pre-registered from record 0064 items 7 and 8 (Part E), before any run: verifies
#include <cerastes.h>

void f1r(int a[][3], int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && cerastes_reads(*q) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_reads(*q) && a[0][0] == 1)
{
  (void)n;
  (void)q;
  a[0][0] = 1;
}

void f2r(int a[][3], int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && cerastes_reads_range(q, 3) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_reads_range(q, 3) && a[0][0] == 1)
{
  (void)n;
  (void)q;
  a[0][0] = 1;
}

int cell_reads(void)
    CERASTES_ENSURES(r, r == 1)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  f1r(m, 1, &m[1][2]);
  return m[0][0];
}

int row_reads(void)
    CERASTES_ENSURES(r, r == 1)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  f2r(m, 1, m[1]);
  return m[0][0];
}

void forward_cell(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 1)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  (void)n;
  f1r(a, 1, &a[1][2]);
}

void forward_row(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 1)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  (void)n;
  f2r(a, 1, a[1]);
}
