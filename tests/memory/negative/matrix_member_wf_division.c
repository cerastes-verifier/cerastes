// expect: error division-by-zero @ L18
// expect: error division-by-zero @ L25
// expect: error division-by-zero @ L32
// expect: error division-by-zero @ L39
// mode: c23, c++20
// twin: tests/memory/matrix_member_wf.c
// Pins: 0064 items 11 and 14 (as struct_member_array_struct_wf_division.c, the guard and the division in one clause's state: question 345): each function proves p->m[0][0] != 0 in its requires and in its ensures and divides in that ensures by another cell, so the division is by zero, once per function. other_col: p->m[0][1]; other_row: p->m[1][0]; other_member: p->n; other_index: p->m[0][j] against p->m[i][0]
// Pre-registered from record 0064 items 11 and 14 (Part E), before any run: division-by-zero at the ensures line of each function
#include <cerastes.h>

struct U {
  int m[2][3];
  int n;
};

int other_col(struct U *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->m[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->m[0][0] != 0 && (r == 0 || r == 10 / p->m[0][1]))  // <- here
{
  return p->m[0][0] * 0;
}

int other_row(struct U *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->m[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->m[0][0] != 0 && (r == 0 || r == 10 / p->m[1][0]))  // <- here
{
  return p->m[0][0] * 0;
}

int other_member(struct U *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && p->m[0][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && p->m[0][0] != 0 && (r == 0 || r == 10 / p->n))  // <- here
{
  return p->m[0][0] * 0;
}

int other_index(struct U *p, int i, int j)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= i && i < 2 && 0 <= j && j < 3 && p->m[i][0] != 0)
    CERASTES_ENSURES(r, cerastes_reads(*p) && 0 <= i && i < 2 && 0 <= j && j < 3 && p->m[i][0] != 0 && (r == 0 || r == 10 / p->m[0][j]))  // <- here
{
  (void)j;
  return p->m[i][0] * 0;
}
