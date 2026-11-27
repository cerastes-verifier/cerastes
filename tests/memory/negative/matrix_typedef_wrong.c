// expect: error postcondition @ L29
// expect: error out-of-bounds @ L36
// expect: error postcondition @ L43
// expect: error postcondition @ L50
// expect: error postcondition @ L57
// expect: error postcondition @ L64
// expect: error check @ L69
// expect: error check @ L70
// mode: c23, c++20
// twin: tests/memory/matrix_typedef.c
// Pins: 0064 items 9 and 13 (the open type gate of Part E; audit round E2a, finding 3): the must-fail twin of matrix_typedef.c, one wrong claim per object that a typedef names, each failing for its own reason at its own line: a local `mat` (r == 7, the cell is 6: `postcondition`), a local `row m[2]` read with `j <= 3` (`out-of-bounds`, the second index), a local `pts` (r == 5), a member `row m[2]` of a structure (r == 5), a member `pts p` (r == 5), a `pts` parameter whose result is `p[2].x` instead of `p[2].y` (`postcondition`), and in `main` a `static mat` and a `static pts` checked against the wrong cell (`check`); the auditor's y05, y07, y08, y23 and the wrong twins of y22, y09 and y24
// Pre-registered from record 0064 items 9 and 13 (Part E; audit round E2a, finding 3), before any run: postcondition at L29; out-of-bounds at L36; postcondition at L43; postcondition at L50; postcondition at L57; postcondition at L64; check at L69; check at L70; corrected once after the first run on j4a (question 160): the two `check` rows were in one function and Silicon reported only the first (L69), so each is in a function of its own, same expect lines; corrected a second time (question 388, no harness run in between): the two functions' checks failed because the value was UNKNOWN in a function (a static is known only in main), so each requires now states the true value (gm[1][1] == 5, gp[2].x == 5) and the check of 6 is wrong, same lines
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
    CERASTES_ENSURES(r, r == 7)
{
  mat m = {{1, 2, 3}, {4, 5, 6}};
  return m[1][2];  // <- here
}

int local_rows(int j)
    CERASTES_REQUIRES(0 <= j && j <= 3)
{
  row m[2] = {{1, 2, 3}, {4, 5, 6}};
  return m[1][j];  // <- here
}

int local_pts(void)
    CERASTES_ENSURES(r, r == 5)
{
  pts p = {{1, 2}, {3, 4}, {5, 6}};
  return p[2].y;  // <- here
}

int member_rows(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{{1, 2, 3}, {4, 5, 6}}, 1};
  return s.m[1][2];  // <- here
}

int member_pts(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct H h = {{{1, 2}, {3, 4}, {5, 6}}, 0};
  return h.p[2].y;  // <- here
}

int param_pts(pts p)
    CERASTES_REQUIRES(cerastes_reads_range(p, 3))
    CERASTES_ENSURES(r, cerastes_reads_range(p, 3) && r == p[2].x)
{
  return p[2].y;  // <- here
}


// Each failing check has a function of its own (L69, L70): Silicon stops a path at its first failing check, so two checks of one function report one error
void check_m(void) CERASTES_REQUIRES(cerastes_owns(gm) && gm[1][1] == 5) { CERASTES_CHECK(gm[1][1] == 6); }  // <- here
void check_p(void) CERASTES_REQUIRES(cerastes_owns(gp) && gp[2].x == 5) { CERASTES_CHECK(gp[2].x == 6); }  // <- here

int main(void)
{
  return gm[0][0] * 0 + gp[0].x * 0;
}
