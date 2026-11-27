// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 (audit round E1, findings 2 and 4; the auditor's j76, j78, j82, j87; models range_struct_forall_beside.c and range_struct_pure_beside.c): a cerastes_forall and a pure application that read a pointer q BESIDE a range over rows (owns_range(a, 2)), framed by q's own atom (reads_range(q, 3), reads(*q)): the contract holds and the body may write the rows and read q; a callee with such a quantifier called with a row of a local matrix (g(m[1])) and beside an owns_range of the first row (g(m, m[1])) verifies. The twins: negative/array_rows_forall_beside_unframed.c, negative/array_rows_pure_beside_unframed.c, negative/array_rows_forall_caller_wrong.c
// Pre-registered from record 0064 item 7 (Part F's neighbour in Part E) as the audit round E1 asks, before any run: verifies
#include <cerastes.h>

CERASTES_PURE bool z(int i, int *p)
{
  return i < 0 || i >= 3 || p[i] == 0;
}

CERASTES_PURE int get(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return *p;
}

int forall_beside(int a[][3], int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && cerastes_reads_range(q, 3) && cerastes_forall(z, q))
    CERASTES_ENSURES(r, cerastes_owns_range(a, 2) && cerastes_reads_range(q, 3) && r == 0)
{
  a[1][1] = 5;
  a[0][0] = 5;
  return q[1];
}

int pure_beside(int a[][3], int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, 2) && cerastes_reads(*q) && get(q) == 0)
    CERASTES_ENSURES(r, cerastes_owns_range(a, 2) && cerastes_reads(*q) && r == 0)
{
  a[1][1] = 5;
  return *q;
}

int callee(int *q)
    CERASTES_REQUIRES(cerastes_reads_range(q, 3) && cerastes_forall(z, q))
    CERASTES_ENSURES(r, cerastes_reads_range(q, 3) && r == 0)
{
  return q[2];
}

int callee_rows(int a[][3], int *q)
    CERASTES_REQUIRES(cerastes_owns_range(a, 1) && cerastes_reads_range(q, 3) && cerastes_forall(z, q))
    CERASTES_ENSURES(r, cerastes_owns_range(a, 1) && cerastes_reads_range(q, 3) && r == 0)
{
  a[0][0] = 5;
  return q[2];
}

int caller_row(void)
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {{1, 2, 3}, {0, 0, 0}};
  return callee(m[1]);
}

int caller_rows(void)
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {{1, 2, 3}, {0, 0, 0}};
  return callee_rows(m, m[1]);
}
