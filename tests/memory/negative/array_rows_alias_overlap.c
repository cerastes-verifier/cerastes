// expect: error permission @ L39
// expect: error permission @ L46
// expect: error permission @ L53
// mode: c23, c++20
// twin: tests/memory/array_rows_alias.c
// Pins: 0064 items 7, 8 and 13 (the alias traps of a range of rows): cell, f1(m, 2, &m[1][2]) owns the cell of row 1 twice (the range holds both rows and so does the cell atom); row, f2(m, 2, m[1]) owns row 1 twice (two ranges); two, g2(m, m) owns row 0 twice. Each is the permission row of the call, once per function
// Pre-registered from record 0064 items 7, 8 and 13 (Part E), before any run: one permission at the call of each function
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
  f1(m, 2, &m[1][2]);  // <- here
  return m[1][2];
}

int row(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  f2(m, 2, m[1]);  // <- here
  return m[1][0];
}

int two(void)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  g2(m, m);  // <- here
  return m[0][0];
}
