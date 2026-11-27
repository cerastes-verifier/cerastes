// expect: error permission @ L62
// expect: error permission @ L69
// expect: error permission @ L77
// expect: error permission @ L84
// expect: error permission @ L91
// expect: error permission @ L98
// mode: c23, c++20
// twin: tests/memory/array_rows_alias_reads.c
// Pins: 0064 items 7 and 8 (audit round E1, finding 4; the auditor's g04, g08, g13, g15, g17 and g19): the pairing rows against a reads site and from a forwarder. A callee that owns the rows a[0..2) of a matrix called with a reads_range or an owns site that may be one object with them (a cell or the row inside the owned rows, the same rows twice, a symbolic cell) is `permission` at the call ("passes ... to ..., which may be one object, once to own it"): one error per function, at the call
// Pre-registered from record 0064 items 7 and 8 (Part E), before any run: one permission error at the call of each function
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

void f1(int a[][3], int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && cerastes_owns(*q) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_owns(*q) && a[0][0] == 1 && *q == 2)
{
  (void)n;
  a[0][0] = 1;
  *q = 2;
}

void f2(int a[][3], int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && cerastes_owns_range(q, 3) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_owns_range(q, 3) && a[0][0] == 1 && q[0] == 2)
{
  (void)n;
  a[0][0] = 1;
  q[0] = 2;
}

void g2(int a[][3], int b[][3], int na, int nb)
    CERASTES_REQUIRES(cerastes_owns_range(a, na) && cerastes_owns_range(b, nb) && na > 0 && nb > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, na) && cerastes_owns_range(b, nb) && a[0][0] == 1 && b[0][0] == 2)
{
  (void)na;
  (void)nb;
  a[0][0] = 1;
  b[0][0] = 2;
}

int cell_reads(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  f1r(m, 2, &m[1][2]);  // <- here
  return m[0][0];
}

int row_reads(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  f2r(m, 2, m[1]);  // <- here
  return m[1][0];
}

void forward_cell(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 1)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  f1(a, n, &a[1][2]);  // <- here
}

void forward_symbolic(int a[][3], int n, int i, int j)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 3 && 0 <= i && i < n && 0 <= j && j < 3)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  f1(a, 2, &a[i][j]);  // <- here
}

void forward_row(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 1)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  f2(a, n, a[1]);  // <- here
}

void forward_twice(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && n > 1)
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  g2(a, a, 1, 1);  // <- here
}
