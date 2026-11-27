// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 13 (the open type gate of Part E; audit round E2a, finding 3: `aggregate_elements_ok` newly accepts objects of a typedef'd matrix and of a typedef'd array of structures, and no test declared one): a LOCAL, a GLOBAL, a MEMBER and a PARAMETER through the typedef of a matrix (`typedef int mat[2][3]`, and the two-step `typedef int row[3]; row m[2]`) and of an array of structures (`typedef struct P pts[3]`); each behaves as the plain declaration it names, and `sizeof(mat)` is 24. Twin: negative/matrix_typedef_wrong.c (a wrong value for each of them, and a second index one too high); the auditor's y04 to y09 and y22 to y25
// Pre-registered from record 0064 items 9 and 13 (Part E; audit round E2a, finding 3), before any run: verifies
#include <cerastes.h>

struct P { int x; int y; };
typedef int mat[2][3];
typedef int row[3];
typedef struct P pts[3];
struct S { row m[2]; int n; };
struct H { pts p; int n; };

static mat gm = {{1, 2, 3}, {4, 5, 6}};
static pts gp = {{1, 2}, {3, 4}, {5, 6}};

int local_mat(void)
    CERASTES_ENSURES(r, r == 24 + 6)
{
  mat m = {{1, 2, 3}, {4, 5, 6}};
  return (int)sizeof(mat) + m[1][2];
}

int local_rows(void)
    CERASTES_ENSURES(r, r == 6)
{
  row m[2] = {{1, 2, 3}, {4, 5, 6}};
  return m[1][2];
}

int local_pts(void)
    CERASTES_ENSURES(r, r == 6)
{
  pts p = {{1, 2}, {3, 4}, {5, 6}};
  return p[2].y;
}

int member_rows(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct S s = {{{1, 2, 3}, {4, 5, 6}}, 1};
  return s.m[1][2];
}

int member_pts(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct H h = {{{1, 2}, {3, 4}, {5, 6}}, 0};
  return h.p[2].y;
}

int param_pts(pts p)
    CERASTES_REQUIRES(cerastes_reads_range(p, 3))
    CERASTES_ENSURES(r, cerastes_reads_range(p, 3) && r == p[2].y)
{
  return p[2].y;
}

int main(void)
{
  CERASTES_CHECK(gm[1][2] == 6);
  CERASTES_CHECK(gp[2].y == 6);
  return gm[0][0] + gp[0].x - 2;
}
