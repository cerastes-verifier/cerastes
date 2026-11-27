// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7, 8 and 13 (Part E; the alias traps of a range of rows, the positives): a callee that owns the first row of a matrix beside a cell of the second (f1(m, 1, &m[1][2])), beside the second row (f2(m, 1, m[1]): owns_range(a, 1) and owns_range(q, 3) are disjoint) and two matrices (g2(m, n)): each passes a disjoint object, so the pairing rows hold. The twins in negative/array_rows_alias_overlap.c pass one object twice
// Pre-registered from record 0064 items 7, 8 and 13 (Part E), before any run: verifies
#include <cerastes.h>

void f1(int a[][3], int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && cerastes_owns(*q) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_owns(*q))
{
  (void)n;
  a[0][0] = 1;
  *q = 2;
}

void f2(int a[][3], int n, int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && cerastes_owns_range(q, 3) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_owns_range(q, 3))
{
  (void)n;
  a[0][0] = 1;
  q[0] = 2;
}

void g2(int a[][3], int b[][3])
    CERASTES_REQUIRES(cerastes_owns_range(a, 1) && cerastes_owns_range(b, 1))
    CERASTES_ENSURES(cerastes_owns_range(a, 1) && cerastes_owns_range(b, 1))
{
  a[0][0] = 1;
  b[0][0] = 2;
}

int cell(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  f1(m, 1, &m[1][2]);
  return m[1][2];
}

int row(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  f2(m, 1, m[1]);
  return m[1][0];
}

int two(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  int n[2][3] = {{0, 0, 0}, {0, 0, 0}};
  g2(m, n);
  return m[0][0] == n[0][0];
}
